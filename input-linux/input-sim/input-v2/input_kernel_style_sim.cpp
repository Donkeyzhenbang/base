// input_kernel_style_sim.cpp
// Userspace simulation of Linux input core with "kernel-style API".
// Provides functions named like kernel input API: input_allocate_device, input_register_device,
// input_register_handler, input_report_key, input_sync, etc.
// Also provides evdev-like user clients that can open and blocking-read events.
// Compile: g++ -std=c++17 input_kernel_style_sim.cpp -O2 -pthread -o input_kernel_style_sim
//
// This is NOT a kernel module. It's a user-space simulation library + demo that closely
// resembles the kernel API naming and semantics for learning/validation.

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <unistd.h>
#include <map>
// -------------------- Basic types --------------------
using namespace std::chrono_literals;
using steady_clock = std::chrono::steady_clock;

enum ev_type_t { EV_SYN = 0, EV_KEY = 1, EV_ABS = 3 };
struct input_event_t {
    std::chrono::steady_clock::time_point time;
    unsigned int type;
    unsigned int code;
    int value;
};

// -------------------- Forward decls --------------------
struct input_dev;
struct input_handler;
struct evdev_client;
struct input_core_sim;

// -------------------- Global core (singleton) --------------------
static std::shared_ptr<input_core_sim> __core_instance; // created in main demo

// -------------------- Simulated Input Core --------------------
struct input_core_sim {
    std::mutex core_mtx;
    std::list<std::shared_ptr<input_dev>> devices;
    std::list<std::shared_ptr<input_handler>> handlers;
    std::atomic<bool> running{true};

    void register_device(std::shared_ptr<input_dev> dev);
    void unregister_device(std::shared_ptr<input_dev> dev);

    void register_handler(std::shared_ptr<input_handler> h);
    void unregister_handler(std::shared_ptr<input_handler> h);

    // Called by device (driver) to report an event
    void report_event(std::shared_ptr<input_dev> dev, const input_event_t &ev);
};

// -------------------- input_dev (kernel-style struct) --------------------
struct input_dev : std::enable_shared_from_this<input_dev> {
    std::string name;
    // capability bits simplified
    bool ev_key_supported = false;
    bool ev_abs_supported = false;

    // reference to core
    input_core_sim *core = nullptr;

    // ctor
    input_dev(const std::string &n = ""): name(n) {}
};

// -------------------- input_handler (kernel-style struct) --------------------
struct input_handler {
    std::string name;
    // match function to decide whether handler attaches to a device (simulate id_table)
    std::function<bool(const input_dev&)> match;

    // callbacks (kernel-like names)
    std::function<int(struct input_handler*, struct input_dev*)> connect; // return 0 ok
    std::function<void(struct input_handler*, struct input_dev*)> disconnect;
    std::function<void(struct input_handler*, struct input_dev*, const input_event_t&)> event;

    // optional internal state
    std::mutex state_mtx;

    input_handler(const std::string &n = ""): name(n) {}
};

// -------------------- Evdev client (user-space process simulator) --------------------
struct evdev_client {
    std::string client_name;
    std::mutex qmtx;
    std::condition_variable qcv;
    std::deque<input_event_t> queue;
    std::atomic<bool> alive{true};

    evdev_client(const std::string &n): client_name(n) {}

    // blocking read simulating read(fd, &ev, sizeof(ev))
    bool blocking_read(input_event_t &out, std::chrono::milliseconds timeout = std::chrono::milliseconds(3000)) {
        std::unique_lock<std::mutex> lk(qmtx);
        qcv.wait_for(lk, timeout, [this]{ return !queue.empty() || !alive.load(); });
        if (!queue.empty()) {
            out = queue.front();
            queue.pop_front();
            return true;
        }
        return false;
    }

    void push_event(const input_event_t &ev) {
        {
            std::lock_guard<std::mutex> lk(qmtx);
            queue.push_back(ev);
        }
        qcv.notify_one();
    }

    void shutdown() { alive.store(false); qcv.notify_all(); }
};

// -------------------- Evdev Handler (simulates kernel evdev handler) --------------------
struct evdev_handler : input_handler {
    // for each attached device we keep a list of clients (each client simulates an open /dev/input/eventX)
    std::mutex clients_mtx;
    std::map<std::string, std::vector<std::shared_ptr<evdev_client>>> dev_clients; // key = device name

    evdev_handler(const std::string &n = "evdev"): input_handler(n) {
        // fill callbacks
        connect = [this](input_handler* h, input_dev* dev) -> int {
            // create empty client list for this device
            std::lock_guard<std::mutex> lk(clients_mtx);
            dev_clients[dev->name] = {};
            std::cout << "[evdev] connected to device: " << dev->name << "\n";
            return 0;
        };
        disconnect = [this](input_handler* h, input_dev* dev) {
            std::lock_guard<std::mutex> lk(clients_mtx);
            auto it = dev_clients.find(dev->name);
            if (it != dev_clients.end()) {
                // shutdown client queues
                for (auto &c : it->second) c->shutdown();
                dev_clients.erase(it);
            }
            std::cout << "[evdev] disconnected from device: " << dev->name << "\n";
        };
        event = [this](input_handler* h, input_dev* dev, const input_event_t &ev) {
            // push event to all clients of this device
            std::lock_guard<std::mutex> lk(clients_mtx);
            auto it = dev_clients.find(dev->name);
            if (it != dev_clients.end()) {
                for (auto &c : it->second) {
                    c->push_event(ev);
                }
            }
        };
    }

    // user-facing: open a client for a device (simulating open("/dev/input/eventX"))
    std::shared_ptr<evdev_client> open_client_for_device(const std::string &devname, const std::string &clientname) {
        std::shared_ptr<evdev_client> c = std::make_shared<evdev_client>(clientname);
        std::lock_guard<std::mutex> lk(clients_mtx);
        auto it = dev_clients.find(devname);
        if (it == dev_clients.end()) {
            // device not attached
            return nullptr;
        }
        it->second.push_back(c);
        std::cout << "[evdev] client '" << clientname << "' opened on device '" << devname << "'\n";
        return c;
    }

    // simulate close
    void close_client_for_device(const std::string &devname, std::shared_ptr<evdev_client> c) {
        std::lock_guard<std::mutex> lk(clients_mtx);
        auto it = dev_clients.find(devname);
        if (it == dev_clients.end()) return;
        auto &vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), c), vec.end());
        c->shutdown();
        std::cout << "[evdev] client '" << c->client_name << "' closed on device '" << devname << "'\n";
    }
};

// -------------------- Logger handler: kernel-like other handler --------------------
struct logger_handler : input_handler {
    logger_handler(const std::string &n = "logger"): input_handler(n) {
        connect = [this](input_handler* h, input_dev* dev)->int {
            std::cout << "[logger] attached to device: " << dev->name << "\n";
            return 0;
        };
        disconnect = [this](input_handler* h, input_dev* dev) {
            std::cout << "[logger] detached from device: " << dev->name << "\n";
        };
        event = [this](input_handler* h, input_dev* dev, const input_event_t &ev){
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ev.time.time_since_epoch()).count();
            std::cout << "[logger] dev='" << dev->name << "' t(ms)=" << ms
                      << " type=" << ev.type << " code=" << ev.code << " val=" << ev.value << "\n";
        };
    }
};

// -------------------- Kernel-style API wrappers --------------------
// allocate/free device
static std::shared_ptr<input_dev> input_allocate_device(const char *name) {
    auto d = std::make_shared<input_dev>(name?name:"unknown");
    return d;
}
static void input_free_device(std::shared_ptr<input_dev> d) {
    // shared_ptr handles it
    (void)d;
}

// register/unregister device
static int input_register_device(std::shared_ptr<input_dev> d) {
    if (!__core_instance) return -1;
    d->core = __core_instance.get();
    __core_instance->register_device(d);
    return 0;
}
static void input_unregister_device(std::shared_ptr<input_dev> d) {
    if (!__core_instance) return;
    __core_instance->unregister_device(d);
    d->core = nullptr;
}

// register/unregister handler
static int input_register_handler(std::shared_ptr<input_handler> h) {
    if (!__core_instance) return -1;
    __core_instance->register_handler(h);
    return 0;
}
static void input_unregister_handler(std::shared_ptr<input_handler> h) {
    if (!__core_instance) return;
    __core_instance->unregister_handler(h);
}

// report helpers (simplified)
static void input_report_key(std::shared_ptr<input_dev> dev, unsigned int code, int value) {
    input_event_t ev;
    ev.time = steady_clock::now();
    ev.type = EV_KEY;
    ev.code = code;
    ev.value = value;
    if (dev && dev->core) dev->core->report_event(dev, ev);
}
static void input_report_abs(std::shared_ptr<input_dev> dev, unsigned int code, int value) {
    input_event_t ev;
    ev.time = steady_clock::now();
    ev.type = EV_ABS;
    ev.code = code;
    ev.value = value;
    if (dev && dev->core) dev->core->report_event(dev, ev);
}
static void input_sync(std::shared_ptr<input_dev> dev) {
    // in this simplified sim, report_event dispatches immediately,
    // so sync can be a no-op or a small sleep to simulate batching boundary.
    std::this_thread::sleep_for(1ms);
}

// -------------------- InputCore implementation --------------------
void input_core_sim::register_device(std::shared_ptr<input_dev> dev) {
    std::lock_guard<std::mutex> lk(core_mtx);
    devices.push_back(dev);
    std::cout << "[core] device registered: " << dev->name << "\n";
    // Immediately try to connect to handlers whose match returns true
    for (auto &h : handlers) {
        if (h->match && h->match(*dev)) {
            // call connect callback in-line (kernel does similar)
            if (h->connect) {
                int rc = h->connect(h.get(), dev.get());
                (void)rc;
            }
        }
    }
}

void input_core_sim::unregister_device(std::shared_ptr<input_dev> dev) {
    std::lock_guard<std::mutex> lk(core_mtx);
    // call disconnect on handlers
    for (auto &h : handlers) {
        if (h->match && h->match(*dev)) {
            if (h->disconnect) h->disconnect(h.get(), dev.get());
        }
    }
    devices.remove(dev);
    std::cout << "[core] device unregistered: " << dev->name << "\n";
}

void input_core_sim::register_handler(std::shared_ptr<input_handler> h) {
    std::lock_guard<std::mutex> lk(core_mtx);
    handlers.push_back(h);
    std::cout << "[core] handler registered: " << h->name << "\n";
    // attach to existing devices that match
    for (auto &dev : devices) {
        if (h->match && h->match(*dev)) {
            if (h->connect) h->connect(h.get(), dev.get());
        }
    }
}

void input_core_sim::unregister_handler(std::shared_ptr<input_handler> h) {
    std::lock_guard<std::mutex> lk(core_mtx);
    // disconnect from devices
    for (auto &dev : devices) {
        if (h->match && h->match(*dev)) {
            if (h->disconnect) h->disconnect(h.get(), dev.get());
        }
    }
    handlers.remove(h);
    std::cout << "[core] handler unregistered: " << h->name << "\n";
}

void input_core_sim::report_event(std::shared_ptr<input_dev> dev, const input_event_t &ev) {
    // copy handlers matching device
    std::vector<std::shared_ptr<input_handler>> matched;
    {
        std::lock_guard<std::mutex> lk(core_mtx);
        for (auto &h : handlers) {
            if (h->match && h->match(*dev)) matched.push_back(h);
        }
    }

    // Dispatch to handlers. In kernel this happens synchronously in input core
    // (and handlers themselves may queue/ wakeup etc). Here we call handler->event()
    // but offload into a small thread per handler to simulate bottom-half safety.
    for (auto &h : matched) {
        auto handler = h;
        auto device = dev;
        input_event_t evcopy = ev;
        // start detached thread to simulate handler bottom half (non-blocking for device)
        std::thread([handler,device,evcopy](){
            // simulate slight handler latencies
            std::this_thread::sleep_for(1ms);
            if (handler->event) handler->event(handler.get(), device.get(), evcopy);
        }).detach();
    }
}

// -------------------- Demo: kernel-style driver + user clients --------------------
int main() {
    // create core singleton
    __core_instance = std::make_shared<input_core_sim>();

    // Create a "kernel-style" device via input_allocate_device()
    auto dev = input_allocate_device("kbd0");
    dev->ev_key_supported = true;
    dev->ev_abs_supported = false;

    // Create handlers: evdev and logger (kernel-side handlers)
    auto evdev_h = std::make_shared<evdev_handler>("evdev");
    evdev_h->match = [](const input_dev &d){ return d.name.find("kbd") != std::string::npos; };

    auto logger_h = std::make_shared<logger_handler>("logger");
    logger_h->match = [](const input_dev &d){ return d.name.find("kbd") != std::string::npos; };

    // Register handlers (kernel-style)
    input_register_handler(evdev_h);
    input_register_handler(logger_h);

    // Register device (kernel-style)
    input_register_device(dev);

    // Simulate user-space opening the evdev device: this calls evdev_h.open_client_for_device()
    auto client1 = evdev_h->open_client_for_device("kbd0", "app-A");
    auto client2 = evdev_h->open_client_for_device("kbd0", "app-B");

    // spawn reader threads simulating user-space processes doing read()
    std::atomic<bool> stop{false};
    auto reader_fn = [&](std::shared_ptr<evdev_client> c, const std::string &tag) {
        while (!stop.load()) {
            input_event_t ev;
            if (c->blocking_read(ev, 5000ms)) {
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ev.time.time_since_epoch()).count();
                std::cout << "["<<tag<<"] read: dev=kbd0 t="<<ms<<" type="<<ev.type
                          <<" code="<<ev.code<<" val="<<ev.value<<"\n";
            } else {
                // timeout or shutdown
            }
        }
        std::cout << "["<<tag<<"] reader exit\n";
    };

    std::thread th1(reader_fn, client1, "ClientA");
    std::thread th2(reader_fn, client2, "ClientB");

    // Kernel-style "driver" code: report key events periodically using kernel-like API
    std::thread drv_thread([&](){
        int cnt = 0;
        bool key_down = false;
        while (cnt < 12) {
            key_down = !key_down;
            std::cout << "[driver] reporting KEY_A " << (key_down?"DOWN":"UP") << "\n";
            input_report_key(dev, 30 /* KEY_A */, key_down?1:0);
            input_sync(dev);
            // occasionally report ABS (not supported here, but demo)
            if (cnt % 3 == 0) {
                input_report_abs(dev, 0 /* ABS misc */, cnt*5);
                input_sync(dev);
            }
            cnt++;
            std::this_thread::sleep_for(700ms);
        }
    });

    // Let system run
    std::this_thread::sleep_for(9000ms);

    // Shutdown sequence (kernel-style)
    std::cout << "[main] shutdown start\n";
    stop.store(true);
    // close clients (evdev)
    evdev_h->close_client_for_device("kbd0", client1);
    evdev_h->close_client_for_device("kbd0", client2);

    // Allow readers to observe shutdown
    th1.join();
    th2.join();

    // Unregister device and handlers like kernel module exit
    input_unregister_device(dev);
    input_unregister_handler(evdev_h);
    input_unregister_handler(logger_h);

    // driver thread join
    drv_thread.join();

    std::cout << "[main] done\n";
    return 0;
}

