// kernel_sim_server.cpp
// Simulated "kernel" server implementing input core with ISR-enqueue + worker dispatch,
// evdev handler exposing IPC-based clients (UNIX domain socket), and logger handler.
//
// Compile: g++ -std=c++17 kernel_sim_server.cpp -O2 -pthread -o kernel_sim_server

#include <arpa/inet.h>
#include <endian.h>      // for htobe64 / be64toh
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ----------------- message framing -----------------
// header: 8 bytes: (uint32_t type, uint32_t len) network order (hton)
struct MsgHeader {
    uint32_t type;
    uint32_t len;
};

// message types (server <-> client)
enum MsgType : uint32_t {
    // client -> server
    MT_OPEN = 1,
    MT_CLOSE = 2,
    MT_IOCTL = 3,       // payload: ioctl_id (u32) + optional payload
    // server -> client
    MT_OPEN_R = 100,    // reply for open {u32 status}
    MT_IOCTL_R = 101,   // reply {u32 status, payload...}
    MT_EVENT = 110,     // payload: event struct
    MT_SHUTDOWN = 111,
};

// ioctl ids
enum IoctlID : uint32_t {
    IO_GET_NAME = 1,
    IO_GET_BITS = 2,   // returns supported event mask
    IO_SET_FILTER = 3, // payload: u32 mask (client-side filter)
};

// event format payload (all in network byte order)
// { u64 ms_since_epoch, u32 type, u32 code, i32 value }

// utility read/write full
static bool write_all(int fd, const void* buf, size_t len) {
    const char* p = (const char*)buf;
    while (len > 0) {
        ssize_t n = write(fd, p, len);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        p += n;
        len -= n;
    }
    return true;
}

static bool read_all(int fd, void* buf, size_t len) {
    char* p = (char*)buf;
    while (len > 0) {
        ssize_t n = read(fd, p, len);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            return false;
        }
        p += n;
        len -= n;
    }
    return true;
}

// ----------------- input model -----------------
enum EventType { EVT_SYN = 0, EVT_KEY = 1, EVT_ABS = 3 };
struct InputEvent {
    uint64_t ms; // ms since epoch
    uint32_t type;
    uint32_t code;
    int32_t value;
};

// forward declaration of EvdevHandler (we will define EvdevHandler after EvdevClient)
class EvdevHandler;

// ----------------- EvdevClient (must be fully defined BEFORE EvdevHandler) ----------
struct EvdevClient {
    std::string name;
    int sock; // connection socket
    std::deque<InputEvent> q;
    std::mutex qmtx;
    std::condition_variable qcv;
    std::atomic<bool> alive{true};
    std::atomic<uint32_t> filter_mask{0xFFFFFFFF}; // default accept all types

    EvdevClient(const std::string &n, int s): name(n), sock(s) {}
    ~EvdevClient() { /* socket closed by owner if not already */ }

    void push_event(const InputEvent &ev) {
        {
            std::lock_guard<std::mutex> lk(qmtx);
            q.push_back(ev);
        }
        qcv.notify_one();
        // send over socket immediately (also)
        send_event_over_socket(ev);
    }

    // send event message to client via socket (MT_EVENT)
    void send_event_over_socket(const InputEvent &ev) {
        if (sock < 0) return;
        MsgHeader h;
        h.type = htonl(MT_EVENT);
        h.len = htonl((uint32_t)(sizeof(ev.ms) + sizeof(ev.type) + sizeof(ev.code) + sizeof(ev.value)));
        if (!write_all(sock, &h, sizeof(h))) { alive.store(false); return; }

        uint64_t ms_net = htobe64(ev.ms);
        uint32_t t_net = htonl(ev.type);
        uint32_t c_net = htonl(ev.code);
        int32_t v_net = htonl(ev.value);

        if (!write_all(sock, &ms_net, sizeof(ms_net))) { alive.store(false); return; }
        if (!write_all(sock, &t_net, sizeof(t_net))) { alive.store(false); return; }
        if (!write_all(sock, &c_net, sizeof(c_net))) { alive.store(false); return; }
        if (!write_all(sock, &v_net, sizeof(v_net))) { alive.store(false); return; }
    }

    // blocking read from queue (unused by socket-mode client, kept for simulation)
    bool blocking_pop(InputEvent &out, std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
        std::unique_lock<std::mutex> lk(qmtx);
        if (!qcv.wait_for(lk, timeout, [this]{ return !q.empty() || !alive.load(); })) return false;
        if (!q.empty()) { out = q.front(); q.pop_front(); return true; }
        return false;
    }

    void shutdown() {
        alive.store(false);
        qcv.notify_all();
        if (sock >= 0) {
            close(sock);
            sock = -1;
        }
    }
};

// ----------------- Evdev handler: keeps list of clients per device name -----------------
class EvdevHandler {
public:
    EvdevHandler() {}
    ~EvdevHandler() {}

    // called when a device connects (device_name)
    void connect_device(const std::string &devname) {
        std::lock_guard<std::mutex> lk(mutex_);
        clients_map_[devname] = std::vector<std::shared_ptr<EvdevClient>>();
        std::cout << "[evdev] connected to device " << devname << "\n";
    }

    // disconnect device: shutdown clients
    void disconnect_device(const std::string &devname) {
        std::lock_guard<std::mutex> lk(mutex_);
        auto it = clients_map_.find(devname);
        if (it != clients_map_.end()) {
            for (auto &c : it->second) {
                if (c) c->shutdown();
            }
            clients_map_.erase(it);
        }
        std::cout << "[evdev] disconnected device " << devname << "\n";
    }

    // open client: add client to device
    bool open_client(const std::string &devname, std::shared_ptr<EvdevClient> c) {
        std::lock_guard<std::mutex> lk(mutex_);
        auto it = clients_map_.find(devname);
        if (it == clients_map_.end()) return false;
        it->second.push_back(c);
        std::cout << "[evdev] client " << c->name << " opened on " << devname << "\n";
        return true;
    }

    // close client
    void close_client(const std::string &devname, std::shared_ptr<EvdevClient> c) {
        std::lock_guard<std::mutex> lk(mutex_);
        auto it = clients_map_.find(devname);
        if (it == clients_map_.end()) return;
        auto &vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), c), vec.end());
        c->shutdown();
        std::cout << "[evdev] client " << c->name << " closed on " << devname << "\n";
    }

    // dispatch event to all clients of device (respect per-client filter)
    void dispatch(const std::string &devname, const InputEvent &ev) {
        std::vector<std::shared_ptr<EvdevClient>> copy;
        {
            std::lock_guard<std::mutex> lk(mutex_);
            auto it = clients_map_.find(devname);
            if (it == clients_map_.end()) return;
            copy = it->second; // copy pointers
        }
        for (auto &c : copy) {
            if (!c) continue;
            uint32_t mask = c->filter_mask.load();
            if ((mask & (1u << ev.type)) == 0) continue; // filtered out
            c->push_event(ev);
        }
    }

private:
    std::mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<EvdevClient>>> clients_map_;
};

// ----------------- Logger handler simply prints event (simulates internal kernel handler) ----------
class LoggerHandler {
public:
    void connect_device(const std::string &devname) {
        std::cout << "[logger] attached to " << devname << "\n";
    }
    void disconnect_device(const std::string &devname) {
        std::cout << "[logger] detached from " << devname << "\n";
    }
    void handle(const std::string &devname, const InputEvent &ev) {
        std::cout << "[logger] dev="<<devname<<" t="<<ev.ms<<" type="<<ev.type
                  <<" code="<<ev.code<<" val="<<ev.value<<"\n";
    }
};

// ----------------- InputCore simulation with enqueue/worker -----------------
class InputCoreSim {
public:
    InputCoreSim(): running_{true} {
        // start worker thread
        worker_ = std::thread([this]{ this->worker_loop(); });
    }
    ~InputCoreSim() {
        running_.store(false);
        cv_.notify_all();
        if (worker_.joinable()) worker_.join();
    }

    // register device: notify handlers (evdev/logger)
    void register_device(const std::string &devname) {
        std::lock_guard<std::mutex> lk(mutex_);
        devices_.push_back(devname);
        evdev_.connect_device(devname);
        logger_.connect_device(devname);
        std::cout << "[core] device registered: " << devname << "\n";
    }

    void unregister_device(const std::string &devname) {
        std::lock_guard<std::mutex> lk(mutex_);
        devices_.remove(devname);
        evdev_.disconnect_device(devname);
        logger_.disconnect_device(devname);
        std::cout << "[core] device unregistered: " << devname << "\n";
    }

    // ISR-style enqueue (fast, non-blocking)
    void input_report_event_isr(const std::string &devname, const InputEvent &ev) {
        {
            std::lock_guard<std::mutex> lk(queue_mtx_);
            event_q_.push_back(std::make_pair(devname, ev));
        }
        cv_.notify_one(); // wake worker
    }

    // worker thread will process events (bottom-half)
    void worker_loop() {
        while (running_.load()) {
            std::pair<std::string, InputEvent> item;
            bool got = false;
            {
                std::unique_lock<std::mutex> lk(queue_mtx_);
                if (event_q_.empty()) {
                    cv_.wait_for(lk, 500ms);
                }
                if (!event_q_.empty()) {
                    item = event_q_.front();
                    event_q_.pop_front();
                    got = true;
                }
            }
            if (!got) continue;
            // process (dispatch)
            dispatch_to_handlers(item.first, item.second);
            // simulate some processing time
            std::this_thread::sleep_for(1ms);
        }
    }

    // dispatch (call handlers)
    void dispatch_to_handlers(const std::string &devname, const InputEvent &ev) {
        // evdev dispatch
        evdev_.dispatch(devname, ev);
        // logger
        logger_.handle(devname, ev);
    }

    // access evdev for client ops
    EvdevHandler &evdev() { return evdev_; }

private:
    std::atomic<bool> running_;
    std::thread worker_;
    std::mutex mutex_;
    std::list<std::string> devices_;

    // event queue & worker coordination
    std::mutex queue_mtx_;
    std::condition_variable cv_;
    std::deque<std::pair<std::string, InputEvent>> event_q_;

    EvdevHandler evdev_;
    LoggerHandler logger_;
};

// -------------------- server side (socket handling) --------------------
static const char *SOCK_PATH = "/tmp/inputsim.sock";
static InputCoreSim *gcore = nullptr;
static int server_fd = -1;
static std::atomic<bool> g_terminate{false};

// handle a connected client socket in a thread
void client_thread_fn(int cfd) {
    // protocol: client sends OPEN (payload deviceName\0 clientName\0), then IOCTL/close
    std::shared_ptr<EvdevClient> evclient = nullptr;
    std::string bound_dev;
    bool running = true;

    while (running && !g_terminate.load()) {
        MsgHeader h;
        if (!read_all(cfd, &h, sizeof(h))) break;
        uint32_t type = ntohl(h.type);
        uint32_t len = ntohl(h.len);
        std::vector<char> payload;
        if (len) {
            payload.resize(len);
            if (!read_all(cfd, payload.data(), len)) break;
        }

        if (type == MT_OPEN) {
            // OPEN payload: deviceName\0clientName\0
            std::string devname, cname;
            if (len == 0) {
                uint32_t status = htonl(1);
                MsgHeader rh { htonl(MT_OPEN_R), htonl(sizeof(status)) };
                write_all(cfd, &rh, sizeof(rh)); write_all(cfd, &status, sizeof(status));
                continue;
            }
            devname = std::string(payload.data());
            cname = std::string(payload.data() + devname.size() + 1);
            evclient = std::make_shared<EvdevClient>(cname, cfd);
            bool ok = gcore->evdev().open_client(devname, evclient);
            uint32_t status = ok ? 0 : 2;
            uint32_t status_n = htonl(status);
            MsgHeader rh { htonl(MT_OPEN_R), htonl(sizeof(status_n)) };
            write_all(cfd, &rh, sizeof(rh));
            write_all(cfd, &status_n, sizeof(status_n));
            if (ok) {
                bound_dev = devname;
            } else {
                evclient->shutdown();
                evclient = nullptr;
            }
        } else if (type == MT_CLOSE) {
            if (evclient && !bound_dev.empty()) {
                gcore->evdev().close_client(bound_dev, evclient);
            }
            running = false;
            break;
        } else if (type == MT_IOCTL) {
            if (len < 4) continue;
            uint32_t iid;
            memcpy(&iid, payload.data(), 4);
            iid = ntohl(iid);
            if (iid == IO_GET_NAME) {
                std::string name = bound_dev;
                MsgHeader rh { htonl(MT_IOCTL_R), htonl(4 + (uint32_t)name.size()+1) };
                uint32_t status = htonl(0);
                write_all(cfd, &rh, sizeof(rh));
                write_all(cfd, &status, 4);
                write_all(cfd, name.c_str(), name.size()+1);
            } else if (iid == IO_GET_BITS) {
                uint32_t mask = (1u<<EVT_KEY) | (1u<<EVT_ABS);
                uint32_t mask_net = htonl(mask);
                MsgHeader rh { htonl(MT_IOCTL_R), htonl(4 + 4) };
                uint32_t status = htonl(0);
                write_all(cfd, &rh, sizeof(rh));
                write_all(cfd, &status, 4);
                write_all(cfd, &mask_net, 4);
            } else if (iid == IO_SET_FILTER) {
                if (len < 8) {
                    uint32_t st = htonl(1);
                    MsgHeader rh { htonl(MT_IOCTL_R), htonl(4) };
                    write_all(cfd, &rh, sizeof(rh)); write_all(cfd, &st, 4);
                } else {
                    uint32_t mask_net;
                    memcpy(&mask_net, payload.data()+4, 4);
                    uint32_t mask = ntohl(mask_net);
                    if (evclient) {
                        evclient->filter_mask.store(mask);
                    }
                    uint32_t st = htonl(0);
                    MsgHeader rh { htonl(MT_IOCTL_R), htonl(4) };
                    write_all(cfd, &rh, sizeof(rh)); write_all(cfd, &st, 4);
                }
            } else {
                uint32_t st = htonl(2);
                MsgHeader rh { htonl(MT_IOCTL_R), htonl(4) };
                write_all(cfd, &rh, sizeof(rh)); write_all(cfd, &st, 4);
            }
        } else {
            // unknown type -> ignore/close
            break;
        }
    }

    if (evclient && !bound_dev.empty()) {
        gcore->evdev().close_client(bound_dev, evclient);
    }
    close(cfd);
    std::cout << "[server] client thread exit\n";
}

// accept loop
void accept_loop(int sfd) {
    while (!g_terminate.load()) {
        struct sockaddr_un client_addr;
        socklen_t clilen = sizeof(client_addr);
        int cfd = accept(sfd, (struct sockaddr*)&client_addr, &clilen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            if (g_terminate.load()) break;
            perror("accept");
            break;
        }
        std::thread(client_thread_fn, cfd).detach();
    }
}

// remove socket path safely
static void cleanup_socket() {
    unlink(SOCK_PATH);
}

// simulate "driver" producing events (ISR-style enqueue)
void driver_thread_fn(InputCoreSim &core) {
    int cnt = 0;
    bool keydown = false;
    while (!g_terminate.load() && cnt < 50) {
        keydown = !keydown;
        InputEvent ev;
        ev.ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        ev.type = EVT_KEY;
        ev.code = 30; // KEY_A
        ev.value = keydown ? 1 : 0;
        core.input_report_event_isr("kbd0", ev);

        if (cnt % 3 == 0) {
            InputEvent ev2;
            ev2.ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            ev2.type = EVT_ABS;
            ev2.code = 0;
            ev2.value = cnt * 5;
            core.input_report_event_isr("kbd0", ev2);
        }

        cnt++;
        std::this_thread::sleep_for(700ms);
    }
}

// signal handler to stop
static void sigint_hdl(int) { g_terminate.store(true); if (server_fd>=0) close(server_fd); }

int main() {
    signal(SIGINT, sigint_hdl);

    InputCoreSim core;
    gcore = &core;

    // prepare socket
    cleanup_socket();
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(server_fd); return 1; }
    if (listen(server_fd, 8) < 0) { perror("listen"); close(server_fd); return 1; }

    std::cout << "[server] listening on " << SOCK_PATH << "\n";

    // register single device "kbd0"
    core.register_device("kbd0");

    // start accept thread
    std::thread acc(accept_loop, server_fd);

    // start driver thread to produce events (ISR-style enqueue)
    std::thread drv(driver_thread_fn, std::ref(core));

    // join until ctrl-c
    while (!g_terminate.load()) {
        std::this_thread::sleep_for(500ms);
    }

    std::cout << "[server] terminating\n";
    // cleanup
    close(server_fd);
    acc.join();
    core.unregister_device("kbd0");
    cleanup_socket();
    drv.join();
    return 0;
}

