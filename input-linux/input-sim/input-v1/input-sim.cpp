// input_sim.cpp
// A simplified userspace simulation of Linux input core, handlers and evdev clients.
// Compile: g++ -std=c++17 input_sim.cpp -O2 -pthread -o input_sim

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// --- simplified event types / codes (like linux input_event) ---
enum EventType { EV_SYN = 0, EV_KEY = 1, EV_ABS = 3 };
enum KeyCode { KEY_A = 30, KEY_B = 48 };
struct InputEvent {
    std::chrono::steady_clock::time_point ts;
    EventType type;
    int code;
    int value;
};

// --- Forward declarations ---
struct InputDevice;
struct InputHandler;
struct InputCore;

// --- InputCore: central dispatcher (device register, handler register, dispatching) ---
struct InputCore {
    std::mutex mtx;
    std::list<std::shared_ptr<InputDevice>> devices;
    std::list<std::shared_ptr<InputHandler>> handlers;
    std::atomic<bool> running{true};

    void register_device(std::shared_ptr<InputDevice> dev);
    void unregister_device(std::shared_ptr<InputDevice> dev);

    void register_handler(std::shared_ptr<InputHandler> h);
    void unregister_handler(std::shared_ptr<InputHandler> h);

    // Called by device (driver) to report an event
    void report_event(std::shared_ptr<InputDevice> dev, const InputEvent &ev);
};

// --- InputHandler: abstract base for handlers (evdev, joydev, custom) ---
struct InputHandler {
    std::string name;
    // simple match by device name substring; real kernel uses id_table
    std::function<bool(const InputDevice&)> match_fn;

    InputHandler(const std::string &n,
                 std::function<bool(const InputDevice&)> m)
        : name(n), match_fn(m) {}
    virtual ~InputHandler() = default;

    // called by input core when device reports an event that matches
    virtual void handle_event(std::shared_ptr<InputDevice> dev, const InputEvent &ev) = 0;
    virtual void start() {}
    virtual void stop() {}
};

// --- InputDevice: simulated device that can report events via core ---
struct InputDevice : std::enable_shared_from_this<InputDevice> {
    std::string name;
    InputCore *core;
    InputDevice(const std::string &n, InputCore *c) : name(n), core(c) {}
    void report(const InputEvent &ev) {
        if (core) core->report_event(shared_from_this(), ev);
    }
};

// --- Evdev client side: each registered client gets a queue and can blocking-read events ---
struct EvdevClient {
    std::string cname;
    std::mutex qmtx;
    std::condition_variable qcv;
    std::deque<InputEvent> q;
    std::atomic<bool> enabled{true};

    EvdevClient(const std::string &n) : cname(n) {}

    // blocking read with timeout; returns false on shutdown/disabled
    bool blocking_read(InputEvent &out, std::chrono::milliseconds timeout = std::chrono::milliseconds(2000)) {
        std::unique_lock<std::mutex> lk(qmtx);
        qcv.wait_for(lk, timeout, [this]{ return !q.empty() || !enabled.load(); });
        if (!q.empty()) {
            out = q.front(); q.pop_front();
            return true;
        }
        return false;
    }

    void push(const InputEvent &ev) {
        {
            std::lock_guard<std::mutex> lk(qmtx);
            q.push_back(ev);
        }
        qcv.notify_one();
    }

    void shutdown() {
        enabled.store(false);
        qcv.notify_all();
    }
};

// --- EvdevHandler: maintains a list of clients (like /dev/input/eventX opens) ---
struct EvdevHandler : InputHandler {
    std::mutex clients_mtx;
    std::vector<std::shared_ptr<EvdevClient>> clients;

    EvdevHandler(const std::string &n, std::function<bool(const InputDevice&)> m)
        : InputHandler(n, m) {}

    // emulate open() by a client, returns a client object
    std::shared_ptr<EvdevClient> open_client(const std::string &client_name) {
        auto c = std::make_shared<EvdevClient>(client_name);
        {
            std::lock_guard<std::mutex> lk(clients_mtx);
            clients.push_back(c);
        }
        return c;
    }

    // emulate close
    void close_client(std::shared_ptr<EvdevClient> c) {
        if (!c) return;
        c->shutdown();
        std::lock_guard<std::mutex> lk(clients_mtx);
        clients.erase(std::remove(clients.begin(), clients.end(), c), clients.end());
    }

    // called by core when event arrives for matched device
    void handle_event(std::shared_ptr<InputDevice> dev, const InputEvent &ev) override {
        // copy event into each client's queue
        std::lock_guard<std::mutex> lk(clients_mtx);
        for (auto &c : clients) {
            c->push(ev);
        }
    }
};

// --- LoggerHandler: an example kernel handler that just logs events (could forward to other subsystems) ---
struct LoggerHandler : InputHandler {
    std::mutex logmtx;
    std::string prefix;
    LoggerHandler(const std::string &n, std::function<bool(const InputDevice&)> m, const std::string &p="")
        : InputHandler(n, m), prefix(p) {}

    void handle_event(std::shared_ptr<InputDevice> dev, const InputEvent &ev) override {
        // print; in kernel would be printk or further processing
        auto t = std::chrono::duration_cast<std::chrono::milliseconds>(ev.ts.time_since_epoch()).count();
        std::lock_guard<std::mutex> lk(logmtx);
        std::cout << "[" << prefix << "] LoggerHandler got event from dev='" << dev->name
                  << "' t(ms)=" << t << " type=" << ev.type << " code=" << ev.code
                  << " val=" << ev.value << "\n";
    }
};

// --- Implementation of InputCore methods ---
void InputCore::register_device(std::shared_ptr<InputDevice> dev) {
    std::lock_guard<std::mutex> lk(mtx);
    devices.push_back(dev);
    std::cout << "[core] device registered: " << dev->name << "\n";
}

void InputCore::unregister_device(std::shared_ptr<InputDevice> dev) {
    std::lock_guard<std::mutex> lk(mtx);
    devices.remove(dev);
    std::cout << "[core] device unregistered: " << dev->name << "\n";
}

void InputCore::register_handler(std::shared_ptr<InputHandler> h) {
    std::lock_guard<std::mutex> lk(mtx);
    handlers.push_back(h);
    h->start();
    std::cout << "[core] handler registered: " << h->name << "\n";
}

void InputCore::unregister_handler(std::shared_ptr<InputHandler> h) {
    std::lock_guard<std::mutex> lk(mtx);
    handlers.remove(h);
    h->stop();
    std::cout << "[core] handler unregistered: " << h->name << "\n";
}

void InputCore::report_event(std::shared_ptr<InputDevice> dev, const InputEvent &ev) {
    // dispatch to handlers that match device. We dispatch asynchronously to avoid blocking device.
    // For simplicity, we call handler->handle_event() directly - this simulates core iterating handlers.
    std::vector<std::shared_ptr<InputHandler>> matched;
    {
        std::lock_guard<std::mutex> lk(mtx);
        for (auto &h : handlers) {
            if (h->match_fn(*dev)) matched.push_back(h);
        }
    }

    // For realism: call each handler in its own deferred task (simulate bottom half)
    // We'll launch small detached tasks; in real kernel there are more controlled contexts.
    for (auto &h : matched) {
        // capture copies
        auto handler = h;
        auto device = dev;
        InputEvent evcopy = ev;
        std::thread([handler, device, evcopy]() {
            // simulate handler processing latency
            std::this_thread::sleep_for(1ms);
            handler->handle_event(device, evcopy);
        }).detach();
    }
}

// --- Utility to create an event ---
InputEvent make_event(EventType t, int code, int value) {
    InputEvent e;
    e.ts = std::chrono::steady_clock::now();
    e.type = t;
    e.code = code;
    e.value = value;
    return e;
}

// --- Demo: create core, device, handlers, clients, and run ---
int main() {
    InputCore core;

    // create a device (like a touchscreen or key device)
    auto dev = std::make_shared<InputDevice>("my_keyboard", &core);
    core.register_device(dev);

    // create evdev handler that matches this device (simple substring match)
    auto evdev = std::make_shared<EvdevHandler>("evdev-handler",
        [](const InputDevice &d){ return d.name.find("keyboard") != std::string::npos; });
    core.register_handler(evdev);

    // create a logger handler that also matches same device
    auto logger = std::make_shared<LoggerHandler>("logger-handler",
        [](const InputDevice &d){ return d.name.find("keyboard") != std::string::npos; },
        "LOG1");
    core.register_handler(logger);

    // simulate two user-space clients opening /dev/input/eventX
    auto clientA = evdev->open_client("app-A");
    auto clientB = evdev->open_client("app-B");

    // client reader threads (simulate blocking read())
    std::atomic<bool> stop{false};

    auto reader_fn = [&](std::shared_ptr<EvdevClient> client, const std::string &tag) {
        while (!stop.load()) {
            InputEvent ev;
            if (client->blocking_read(ev, 3000ms)) {
                // print event
                auto tms = std::chrono::duration_cast<std::chrono::milliseconds>(ev.ts.time_since_epoch()).count();
                std::cout << "[" << tag << "] got event t=" << tms
                          << " type=" << ev.type << " code=" << ev.code
                          << " val=" << ev.value << "\n";
            } else {
                // timeout or shutdown
            }
        }
        std::cout << "[" << tag << "] exiting reader\n";
    };

    std::thread tA(reader_fn, clientA, "ClientA");
    std::thread tB(reader_fn, clientB, "ClientB");

    // simulate device producing events periodically
    std::thread dev_thread([&]() {
        int cnt = 0;
        bool key_state = false;
        while (cnt < 10) {
            // alternate KEY press/release and ABS value
            if (cnt % 2 == 0) {
                key_state = !key_state;
                dev->report(make_event(EV_KEY, KEY_A, key_state ? 1 : 0));
            } else {
                int absval = 100 + cnt*5;
                dev->report(make_event(EV_ABS, 0, absval));
            }
            cnt++;
            std::this_thread::sleep_for(700ms);
        }
    });

    // run for a while
    std::this_thread::sleep_for(9000ms);

    // shutdown
    std::cout << "[main] shutting down...\n";
    stop.store(true);

    // close evdev clients
    evdev->close_client(clientA);
    evdev->close_client(clientB);

    // give readers time to exit
    tA.join();
    tB.join();

    // unregister handlers/devices
    core.unregister_handler(evdev);
    core.unregister_handler(logger);
    core.unregister_device(dev);

    dev_thread.join();

    std::cout << "[main] all done\n";
    return 0;
}

