/**
一、空调系统题目：智能温控系统
题目要求
1.设计一个智能温控系统，包含以下组件：
2.温度传感器：实时监测环境温度，可同时被多个控制器订阅
3.空调控制器：根据当前温度和目标温度控制空调运行
4.用户面板：显示当前状态，允许用户调整目标温度
5.节能管理器：根据时间自动调整目标温度以节省能源

需求：

- 温度传感器每秒采集一次温度（模拟数据）
- 空调控制器需要知道温度来决策，但不应直接依赖传感器
- 用户面板需要显示温度和空调状态
- 节能管理器需要根据时间调整目标温度（白天26°C，晚上24°C）
- 系统可扩展：未来可能添加湿度传感器、远程控制等
 */

/* 架构图
┌─────────────────┐   事件    ┌─────────────┐
│ TemperatureSensor├───────────►             │
└─────────────────┘           │             │
┌─────────────────┐   事件    │             │
│   ACController   ├───────────► Event Bus  │
└─────────────────┘           │             │
┌─────────────────┐   事件    │             │
│  EnergyManager   ├───────────►             │
└─────────────────┘           │             │
┌─────────────────┐   事件    │             │
│    UserPanel     ├───────────►             │
└─────────────────┘           └─────────────┘
        ▲                            │
        │                            │ 事件分发
        │                            │
        │                            ▼
        │                      ┌─────────────┐
        └──────────────────────┤   订阅者    │
                               └─────────────┘
! 设计方案：温度传感器、节能管理器独占一个线程，也即publish线程 根据环境温度实时上报；事件总线一个线程，负责后台田间subscriber和根据publish线程发布的信息同步订阅信息的handler；最后主线程负责UI界面；对于ACcontroller只订阅服务，根据温度变化实行空调温度相关操作，比如这里ACController我们只需要传入EventBus即可，其内部以及subscribe好了对应的handler相当于回调函数，那我们只需要在EventBus线程中根据event变化类型去调用handler即可实现对于底层温度风机的间接控制

*/

#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <functional>
#include <random>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <atomic>

enum class ACSystemEventType {
    TEMPERATURE_CHANGED,    // 温度变化
    TARGET_TEMP_CHANGED,    // 目标温度变化
    AC_MODE_CHANGED,        // 空调模式变化
    ENERGY_MODE_CHANGED,    // 节能模式变化
    SYSTEM_STATE_UPDATED    // 系统状态更新
};


struct ACSystemEvent {
    ACSystemEventType type;
    union {
        double temperature;    // 当前温度
        double target_temp;    // 目标温度
        int mode;              // 模式
    } data;
    std::string source;        // 事件来源
    std::time_t timestamp;     // 时间戳
};

class EventBus {
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<ACSystemEvent> events_;
    std::map<ACSystemEventType, std::vector<std::function<void(const ACSystemEvent&)>>> subscribers_;
    bool running_ = true; //! 这里不要默认赋初值

public:

    // 订阅事件
    void subscribe(ACSystemEventType type, std::function<void(const ACSystemEvent&)> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_[type].push_back(handler);
    }
    // 发布事件
    void publish(const ACSystemEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.push(event);
        cv_.notify_one();
    }

    // 处理事件（运行在专用线程）
    void process_events() {
        while(running_){
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] {return !events_.empty()|| !running_; });
            
            if(!running_ && events_.empty()) break;

            if(!events_.empty()){
                ACSystemEvent event = events_.front();
                events_.pop();
                lock.unlock();
                auto it = subscribers_.find(event.type);
                if(it != subscribers_.end()) {
                    for(const auto& handler : it->second){
                        try{
                            handler(event);
                        } catch(...){
                            // 处理异常
                        }
                    }
                }
            }
        }
    }

    void stop() {
        running_ = false;
        cv_.notify_all();
    }
};

// ==================== 温度传感器 ====================
class TemperatureSensor {
private:
    EventBus* event_bus_;
    std::thread sensor_thread_;
    std::atomic<bool> running_{true};
    std::string sensor_id_;
    double current_temp_;
    mutable std::mutex temp_mutex_;
    std::normal_distribution<> temp_distribution_;
    std::mt19937 rng_;
public:
    TemperatureSensor(EventBus* bus, const std::string& id, double initial_temp = 25.0)
        : event_bus_(bus), sensor_id_(id), current_temp_(initial_temp),
          temp_distribution_(initial_temp, 2.0), rng_(std::random_device{}()) {
        
        sensor_thread_ = std::thread([this] { this->sensor_loop(); });
    }

    ~TemperatureSensor() {
        running_ = false;
        if (sensor_thread_.joinable()) {
            sensor_thread_.join();
        }
    }
    
    double get_current_temperature() const {
        std::lock_guard<std::mutex> lock(temp_mutex_);
        return current_temp_;
    }
private:
    void sensor_loop() {
        while(running_){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            double new_temp;
            // 模拟温度变化
            {
                std::lock_guard<std::mutex> lock(temp_mutex_);
                new_temp = temp_distribution_(rng_);
                // 添加一些趋势
                double delta = (rng_() % 100) / 100.0 - 0.5;
                current_temp_ += delta;
                if (current_temp_ < 15.0) current_temp_ = 15.0;
                if (current_temp_ > 35.0) current_temp_ = 35.0;
                new_temp = current_temp_;
            }
            //发布温度变化事件
            {
                if(event_bus_){
                    event_bus_->publish({
                        ACSystemEventType::TEMPERATURE_CHANGED,
                        {.temperature = new_temp},
                        sensor_id_,
                        std::time(nullptr)
                    });
                }
            }
        }
    }
};

// ==================== 空调模式 ====================
enum class ACMode {
    OFF,
    COOLING,
    HEATING,
    FAN_ONLY,
    AUTO
};

// ==================== 空调控制器 ====================
class ACController {
private:
    EventBus* event_bus_;
    std::string controller_id_;
    ACMode current_mode_;
    double target_temperature_;
    double current_temperature_;
    double hysteresis_;  // 迟滞值，防止频繁切换
    
    // 状态统计
    int total_runtime_minutes_;
    double energy_consumed_kwh_;
public:
    ACController(EventBus* bus, const std::string& id, double initial_target = 24.0)
        : event_bus_(bus), controller_id_(id), current_mode_(ACMode::OFF),
          target_temperature_(initial_target), current_temperature_(25.0),
          hysteresis_(0.5), total_runtime_minutes_(0), energy_consumed_kwh_(0.0) {
        
        // 订阅温度变化事件
        if (event_bus_) {
            event_bus_->subscribe(ACSystemEventType::TEMPERATURE_CHANGED,
                [this](const ACSystemEvent& event) {
                    this->on_temperature_changed(event.data.temperature);
                });
            
            event_bus_->subscribe(ACSystemEventType::TARGET_TEMP_CHANGED,
                [this](const ACSystemEvent& event) {
                    this->on_target_temp_changed(event.data.target_temp);
                });
        }
    }
    
    void set_target_temperature(double temp) {
        if (temp >= 16.0 && temp <= 30.0) {
            target_temperature_ = temp;
            
            if (event_bus_) {
                event_bus_->publish({
                    ACSystemEventType::TARGET_TEMP_CHANGED,
                    { .target_temp = temp },
                    controller_id_,
                    std::time(nullptr)
                });
            }
            
            regulate_temperature();
        }
    }
    
    double get_target_temperature() const { return target_temperature_; }
    ACMode get_current_mode() const { return current_mode_; }
    double get_energy_consumed() const { return energy_consumed_kwh_; }
    int get_runtime() const { return total_runtime_minutes_; }
    
private:
    void on_temperature_changed(double temperature) {
        current_temperature_ = temperature;
        regulate_temperature();
    }
    
    void on_target_temp_changed(double target_temp) {
        target_temperature_ = target_temp;
        regulate_temperature();
    }
    
    void regulate_temperature() {
        ACMode new_mode = current_mode_;
        
        if (current_mode_ == ACMode::OFF || current_mode_ == ACMode::AUTO) {
            if (current_temperature_ > target_temperature_ + hysteresis_) {
                new_mode = ACMode::COOLING;
            } else if (current_temperature_ < target_temperature_ - hysteresis_) {
                new_mode = ACMode::HEATING;
            } else {
                new_mode = ACMode::OFF;
            }
        } else if (current_mode_ == ACMode::COOLING) {
            if (current_temperature_ <= target_temperature_) {
                new_mode = ACMode::OFF;
            }
        } else if (current_mode_ == ACMode::HEATING) {
            if (current_temperature_ >= target_temperature_) {
                new_mode = ACMode::OFF;
            }
        }
        
        if (new_mode != current_mode_) {
            current_mode_ = new_mode;
            
            if (event_bus_) {
                event_bus_->publish({
                    ACSystemEventType::AC_MODE_CHANGED,
                    { .mode = static_cast<int>(new_mode) },
                    controller_id_,
                    std::time(nullptr)
                });
            }
            
            // 更新状态统计
            if (new_mode != ACMode::OFF) {
                total_runtime_minutes_++;
                // 简化能耗计算：制冷1.5kW，制热2.0kW，风扇0.1kW
                double power_kw = 0.0;
                switch (new_mode) {
                    case ACMode::COOLING: power_kw = 1.5; break;
                    case ACMode::HEATING: power_kw = 2.0; break;
                    case ACMode::FAN_ONLY: power_kw = 0.1; break;
                    default: break;
                }
                energy_consumed_kwh_ += power_kw / 60.0; // 按分钟计算
            }
        }
    }
};

// ==================== 节能管理器 ====================
class EnergyManager {
private:
    EventBus* event_bus_;
    std::string manager_id_;
    double daytime_target_;
    double nighttime_target_;
    bool is_daytime_;
    
    std::thread schedule_thread_;
    std::atomic<bool> running_{true};
    
public:
    EnergyManager(EventBus* bus, const std::string& id, 
                  double day_temp = 26.0, double night_temp = 24.0)
        : event_bus_(bus), manager_id_(id),
          daytime_target_(day_temp), nighttime_target_(night_temp),
          is_daytime_(true) {
        
        schedule_thread_ = std::thread([this] { this->schedule_loop(); });
        
        // 初始设置
        if (event_bus_) {
            event_bus_->publish({
                ACSystemEventType::TARGET_TEMP_CHANGED,
                { .target_temp = daytime_target_ },
                manager_id_,
                std::time(nullptr)
            });
        }
    }
    
    ~EnergyManager() {
        running_ = false;
        if (schedule_thread_.joinable()) {
            schedule_thread_.join();
        }
    }
    
    void set_temperatures(double day_temp, double night_temp) {
        daytime_target_ = day_temp;
        nighttime_target_ = night_temp;
        check_and_adjust();
    }
    
private:
    void schedule_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::minutes(1));
            check_and_adjust();
        }
    }
    
    void check_and_adjust() {
        std::time_t now = std::time(nullptr);
        std::tm* local_time = std::localtime(&now);
        int hour = local_time->tm_hour;
        
        bool new_daytime = (hour >= 8 && hour < 22); // 8:00-22:00为白天
        
        if (new_daytime != is_daytime_) {
            is_daytime_ = new_daytime;
            double target_temp = is_daytime_ ? daytime_target_ : nighttime_target_;
            
            if (event_bus_) {
                event_bus_->publish({
                    ACSystemEventType::ENERGY_MODE_CHANGED,
                    { .target_temp = target_temp },
                    manager_id_,
                    now
                });
                
                event_bus_->publish({
                    ACSystemEventType::TARGET_TEMP_CHANGED,
                    { .target_temp = target_temp },
                    manager_id_,
                    now
                });
            }
        }
    }
};


// ==================== 用户面板 ====================
class UserPanel {
private:
    EventBus* event_bus_;
    std::string panel_id_;
    
    double current_temperature_;
    double target_temperature_;
    ACMode ac_mode_;
    bool energy_saving_mode_;
    
    mutable std::mutex display_mutex_;
    
public:
    UserPanel(EventBus* bus, const std::string& id)
        : event_bus_(bus), panel_id_(id),
          current_temperature_(25.0), target_temperature_(24.0),
          ac_mode_(ACMode::OFF), energy_saving_mode_(true) {
        
        if (event_bus_) {
            // 订阅所有相关事件
            event_bus_->subscribe(ACSystemEventType::TEMPERATURE_CHANGED,
                [this](const ACSystemEvent& event) {
                    this->on_temperature_changed(event.data.temperature);
                });
            
            event_bus_->subscribe(ACSystemEventType::TARGET_TEMP_CHANGED,
                [this](const ACSystemEvent& event) {
                    this->on_target_temp_changed(event.data.target_temp);
                });
            
            event_bus_->subscribe(ACSystemEventType::AC_MODE_CHANGED,
                [this](const ACSystemEvent& event) {
                    this->on_ac_mode_changed(static_cast<ACMode>(event.data.mode));
                });
            
            event_bus_->subscribe(ACSystemEventType::ENERGY_MODE_CHANGED,
                [this](const ACSystemEvent& event) {
                    this->on_energy_mode_changed(event.data.target_temp);
                });
        }
    }
    
    void display() const {
        std::lock_guard<std::mutex> lock(display_mutex_);
        
        std::system("clear");
        std::cout << "=== 智能温控系统 ===" << std::endl;
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "当前温度: " << current_temperature_ << "°C" << std::endl;
        std::cout << "目标温度: " << target_temperature_ << "°C" << std::endl;
        std::cout << "空调模式: ";
        switch (ac_mode_) {
            case ACMode::OFF: std::cout << "关机"; break;
            case ACMode::COOLING: std::cout << "制冷"; break;
            case ACMode::HEATING: std::cout << "制热"; break;
            case ACMode::FAN_ONLY: std::cout << "送风"; break;
            case ACMode::AUTO: std::cout << "自动"; break;
        }
        std::cout << std::endl;
        std::cout << "节能模式: " << (energy_saving_mode_ ? "开启" : "关闭") << std::endl;
        std::cout << "===================" << std::endl;
        std::cout << "操作选项:" << std::endl;
        std::cout << "1. 升高目标温度" << std::endl;
        std::cout << "2. 降低目标温度" << std::endl;
        std::cout << "3. 切换节能模式" << std::endl;
        std::cout << "4. 显示能耗统计" << std::endl;
        std::cout << "5. 退出系统" << std::endl;
        std::cout << "请选择: ";
    }
    
    void adjust_target_temperature(double delta) {
        double new_target = target_temperature_ + delta;
        if (new_target >= 16.0 && new_target <= 30.0) {
            if (event_bus_) {
                event_bus_->publish({
                    ACSystemEventType::TARGET_TEMP_CHANGED,
                    { .target_temp = new_target },
                    panel_id_,
                    std::time(nullptr)
                });
            }
        }
    }
    
    void toggle_energy_saving() {
        energy_saving_mode_ = !energy_saving_mode_;
        std::cout << "节能模式 " << (energy_saving_mode_ ? "开启" : "关闭") << std::endl;
    }
    
private:
    void on_temperature_changed(double temp) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        current_temperature_ = temp;
    }
    
    void on_target_temp_changed(double temp) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        target_temperature_ = temp;
    }
    
    void on_ac_mode_changed(ACMode mode) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        ac_mode_ = mode;
    }
    
    void on_energy_mode_changed(double target_temp) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        energy_saving_mode_ = true;
        target_temperature_ = target_temp;
    }
};

// ==================== 主控制系统 ====================
class ACSystem {
private:
    std::unique_ptr<EventBus> event_bus_;
    std::unique_ptr<TemperatureSensor> temperature_sensor_;
    std::unique_ptr<ACController> ac_controller_;
    std::unique_ptr<EnergyManager> energy_manager_;
    std::unique_ptr<UserPanel> user_panel_;
    
    std::thread event_thread_;
    std::atomic<bool> running_{true};
    
public:
    ACSystem() {
        // 创建事件总线
        event_bus_ = std::make_unique<EventBus>();
        
        // 创建组件
        temperature_sensor_ = std::make_unique<TemperatureSensor>(event_bus_.get(), "Sensor1", 25.0);
        ac_controller_ = std::make_unique<ACController>(event_bus_.get(), "AC1", 24.0);
        energy_manager_ = std::make_unique<EnergyManager>(event_bus_.get(), "EnergyMgr", 26.0, 24.0);
        user_panel_ = std::make_unique<UserPanel>(event_bus_.get(), "Panel1");
        
        // 启动事件处理线程
        event_thread_ = std::thread([this] { 
            this->event_bus_->process_events(); 
        });
    }
    
    ~ACSystem() {
        running_ = false;
        event_bus_->stop();
        if (event_thread_.joinable()) {
            event_thread_.join();
        }
    }
    
    void run() {
        while (running_) {
            user_panel_->display();
            
            int choice;
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    user_panel_->adjust_target_temperature(0.5);
                    break;
                case 2:
                    user_panel_->adjust_target_temperature(-0.5);
                    break;
                case 3:
                    user_panel_->toggle_energy_saving();
                    break;
                case 4:
                    show_energy_stats();
                    break;
                case 5:
                    running_ = false;
                    std::cout << "系统正在关闭..." << std::endl;
                    break;
                default:
                    std::cout << "无效选择，请重试" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    break;
            }
        }
    }
    
private:
    void show_energy_stats() const {
        std::system("clear");
        std::cout << "=== 能耗统计 ===" << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "累计运行时间: " << ac_controller_->get_runtime() << " 分钟" << std::endl;
        std::cout << "累计能耗: " << ac_controller_->get_energy_consumed() << " kWh" << std::endl;
        std::cout << "当前目标温度: " << ac_controller_->get_target_temperature() << "°C" << std::endl;
        std::cout << "当前空调模式: ";
        switch (ac_controller_->get_current_mode()) {
            case ACMode::OFF: std::cout << "关机"; break;
            case ACMode::COOLING: std::cout << "制冷"; break;
            case ACMode::HEATING: std::cout << "制热"; break;
            default: std::cout << "其他"; break;
        }
        std::cout << std::endl;
        std::cout << "按回车键返回...";
        std::cin.ignore();
        std::cin.get();
    }
};


// ==================== 主函数 ====================
int main() {
    try {
        ACSystem system;
        system.run();
        std::cout << "系统已关闭" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "系统错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}