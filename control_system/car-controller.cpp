/**
车控系统题目要求
假设我们有一个智能车辆控制系统，包含以下组件：
引擎（Engine）：负责控制引擎的转速（RPM）和油门。
变速箱（Transmission）：负责换挡，根据当前车速和引擎转速决定最佳档位。
车速传感器（SpeedSensor）：监测车速并通知监听器。
驾驶模式控制器（DrivingModeController）：根据驾驶模式（经济、运动）调整换挡策略和引擎响应。

要求：

引擎可以设置油门开度（0-100%），并计算当前转速（模拟：转速 = 油门开度 * 100 + 随机波动）。
车速传感器每2秒读取一次车速（模拟：车速 = 当前档位 * 引擎转速 / 100 + 随机波动）。
变速箱根据当前车速、引擎转速和驾驶模式进行换挡：
经济模式：提前升档（转速>2000升档，转速<1500降档）。
运动模式：延迟升档（转速>4000升档，转速<3000降档）。
驾驶模式控制器允许用户切换驾驶模式，并通知相关组件。
系统需要支持未来添加新的组件（如制动系统、牵引力控制等）。
设计一个解耦的系统，使用事件总线和策略模式，确保组件之间不直接依赖。
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
#include <cmath>
#include <sstream>
#include <algorithm>
#include <atomic>

// ==================== 事件定义 ====================
enum class VehicleEventType {
    THROTTLE_POSITION_CHANGED,    // 油门位置变化
    BRAKE_PEDAL_PRESSED,          // 刹车踏板按下
    ENGINE_RPM_CHANGED,           // 引擎转速变化
    VEHICLE_SPEED_CHANGED,        // 车速变化
    GEAR_SHIFT_REQUESTED,         // 换挡请求
    GEAR_SHIFT_COMPLETED,         // 换挡完成
    DRIVING_MODE_CHANGED,         // 驾驶模式变化
    SYSTEM_ALERT                  // 系统警报
};

struct VehicleEvent {
    VehicleEventType type;
    union {
        double throttle_position;  // 油门位置 0-100%
        double brake_pressure;     // 刹车压力 0-100%
        double engine_rpm;         // 引擎转速 RPM
        double vehicle_speed;      // 车速 km/h
        int gear_number;           // 档位
        int driving_mode;          // 驾驶模式
    } data;
    std::string source;
    std::chrono::system_clock::time_point timestamp;
};

// ==================== 事件总线 ====================
class VehicleEventBus {
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<VehicleEvent> events_;
    std::map<VehicleEventType, std::vector<std::function<void(const VehicleEvent&)>>> subscribers_;
    std::atomic<bool> running_{true};
    
public:
    void subscribe(VehicleEventType type, std::function<void(const VehicleEvent&)> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_[type].push_back(handler);
    }
    
    void publish(const VehicleEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.push(event);
        cv_.notify_one();
    }
    
    void process_events() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !events_.empty() || !running_; });
            
            if (!running_ && events_.empty()) break;
            
            if (!events_.empty()) {
                VehicleEvent event = events_.front();
                events_.pop();
                lock.unlock();
                
                auto it = subscribers_.find(event.type);
                if (it != subscribers_.end()) {
                    for (const auto& handler : it->second) {
                        try {
                            handler(event);
                        } catch (...) {
                            // 异常处理
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

// ==================== 驾驶模式 ====================
enum class DrivingMode {
    ECONOMY,      // 经济模式
    NORMAL,       // 普通模式
    SPORT,        // 运动模式
    SNOW,         // 雪地模式
    MANUAL        // 手动模式
};

// ==================== 引擎控制器 ====================
class EngineController {
private:
    VehicleEventBus* event_bus_;
    std::string engine_id_;
    
    // 引擎状态
    double throttle_position_;    // 0-100%
    double current_rpm_;          // 当前转速
    double max_rpm_;              // 最大转速
    double idle_rpm_;             // 怠速转速
    bool is_running_;             // 是否运行
    
    // 模式相关参数
    double response_factor_;      // 响应系数
    
    std::thread engine_thread_;
    std::atomic<bool> running_{true};
    mutable std::mutex engine_mutex_;
    
public:
    EngineController(VehicleEventBus* bus, const std::string& id)
        : event_bus_(bus), engine_id_(id),
          throttle_position_(0.0), current_rpm_(800.0),
          max_rpm_(6000.0), idle_rpm_(800.0),
          is_running_(true), response_factor_(1.0) {
        
        // 订阅事件
        if (event_bus_) {
            event_bus_->subscribe(VehicleEventType::THROTTLE_POSITION_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_throttle_changed(event.data.throttle_position);
                });
            
            event_bus_->subscribe(VehicleEventType::BRAKE_PEDAL_PRESSED,
                [this](const VehicleEvent& event) {
                    this->on_brake_pressed(event.data.brake_pressure);
                });
            
            event_bus_->subscribe(VehicleEventType::DRIVING_MODE_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_driving_mode_changed(static_cast<DrivingMode>(event.data.driving_mode));
                });
        }
        
        // 启动引擎模拟线程
        engine_thread_ = std::thread([this] { this->engine_simulation_loop(); });
    }
    
    ~EngineController() {
        running_ = false;
        if (engine_thread_.joinable()) {
            engine_thread_.join();
        }
    }
    
    double get_current_rpm() const {
        std::lock_guard<std::mutex> lock(engine_mutex_);
        return current_rpm_;
    }
    
    double get_throttle_position() const {
        std::lock_guard<std::mutex> lock(engine_mutex_);
        return throttle_position_;
    }
    
    void set_throttle(double throttle) {
        if (throttle < 0.0) throttle = 0.0;
        if (throttle > 100.0) throttle = 100.0;
        
        std::lock_guard<std::mutex> lock(engine_mutex_);
        throttle_position_ = throttle;
        
        if (event_bus_) {
            event_bus_->publish({
                VehicleEventType::THROTTLE_POSITION_CHANGED,
                { .throttle_position = throttle },
                engine_id_,
                std::chrono::system_clock::now()
            });
        }
    }
    
private:
    void on_throttle_changed(double throttle) {
        std::lock_guard<std::mutex> lock(engine_mutex_);
        throttle_position_ = throttle;
    }
    
    void on_brake_pressed(double brake_pressure) {
        std::lock_guard<std::mutex> lock(engine_mutex_);
        // 刹车时自动减少油门
        if (brake_pressure > 20.0) {
            throttle_position_ *= (1.0 - brake_pressure / 200.0);
            if (throttle_position_ < 0.0) throttle_position_ = 0.0;
        }
    }
    
    void on_driving_mode_changed(DrivingMode mode) {
        std::lock_guard<std::mutex> lock(engine_mutex_);
        switch (mode) {
            case DrivingMode::ECONOMY:
                response_factor_ = 0.7;
                break;
            case DrivingMode::NORMAL:
                response_factor_ = 1.0;
                break;
            case DrivingMode::SPORT:
                response_factor_ = 1.3;
                break;
            case DrivingMode::SNOW:
                response_factor_ = 0.5;
                break;
            case DrivingMode::MANUAL:
                response_factor_ = 1.0;
                break;
        }
    }
    
    void engine_simulation_loop() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::normal_distribution<> noise_dist(0.0, 20.0);
        
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10Hz
            
            double rpm;
            {
                std::lock_guard<std::mutex> lock(engine_mutex_);
                
                if (!is_running_) {
                    current_rpm_ = 0.0;
                    continue;
                }
                
                // 计算目标转速：怠速 + 油门控制的转速范围
                double target_rpm = idle_rpm_ + 
                    (throttle_position_ / 100.0) * (max_rpm_ - idle_rpm_) * response_factor_;
                
                // 模拟引擎响应延迟
                double acceleration = (target_rpm - current_rpm_) * 0.1;
                current_rpm_ += acceleration;
                
                // 添加随机噪声
                current_rpm_ += noise_dist(rng);
                
                // 限制范围
                if (current_rpm_ < idle_rpm_) current_rpm_ = idle_rpm_;
                if (current_rpm_ > max_rpm_) current_rpm_ = max_rpm_;
                
                rpm = current_rpm_;
            }
            
            // 发布转速变化事件
            if (event_bus_ && running_) {
                event_bus_->publish({
                    VehicleEventType::ENGINE_RPM_CHANGED,
                    { .engine_rpm = rpm },
                    engine_id_,
                    std::chrono::system_clock::now()
                });
            }
        }
    }
};

// ==================== 变速箱控制器 ====================
class TransmissionController {
private:
    VehicleEventBus* event_bus_;
    std::string transmission_id_;
    
    // 变速箱状态
    int current_gear_;           // 当前档位 (0=空挡, 1-6=前进档, -1=倒档)
    int max_gear_;               // 最大档位
    bool is_automatic_;          // 是否自动模式
    
    // 换挡参数（不同模式不同）
    struct ShiftPoint {
        double upshift_rpm;      // 升挡转速
        double downshift_rpm;    // 降挡转速
    };
    
    std::map<DrivingMode, ShiftPoint> shift_points_;
    DrivingMode current_mode_;
    
    // 车速相关
    double current_speed_;       // km/h
    double wheel_radius_;        // 车轮半径
    
public:
    TransmissionController(VehicleEventBus* bus, const std::string& id)
        : event_bus_(bus), transmission_id_(id),
          current_gear_(1), max_gear_(6), is_automatic_(true),
          current_mode_(DrivingMode::NORMAL), current_speed_(0.0),
          wheel_radius_(0.3) {
        
        // 初始化换挡点
        initialize_shift_points();
        
        // 订阅事件
        if (event_bus_) {
            event_bus_->subscribe(VehicleEventType::ENGINE_RPM_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_engine_rpm_changed(event.data.engine_rpm);
                });
            
            event_bus_->subscribe(VehicleEventType::VEHICLE_SPEED_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_vehicle_speed_changed(event.data.vehicle_speed);
                });
            
            event_bus_->subscribe(VehicleEventType::DRIVING_MODE_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_driving_mode_changed(static_cast<DrivingMode>(event.data.driving_mode));
                });
        }
    }
    
    void shift_gear(int gear) {
        if (gear >= -1 && gear <= max_gear_) {
            if (gear != current_gear_) {
                current_gear_ = gear;
                
                if (event_bus_) {
                    event_bus_->publish({
                        VehicleEventType::GEAR_SHIFT_COMPLETED,
                        { .gear_number = gear },
                        transmission_id_,
                        std::chrono::system_clock::now()
                    });
                }
            }
        }
    }
    
    int get_current_gear() const { return current_gear_; }
    
private:
    void initialize_shift_points() {
        // 经济模式：提前换挡
        shift_points_[DrivingMode::ECONOMY] = { 2000.0, 1500.0 };
        // 普通模式
        shift_points_[DrivingMode::NORMAL] = { 2500.0, 1800.0 };
        // 运动模式：延迟换挡
        shift_points_[DrivingMode::SPORT] = { 4000.0, 3000.0 };
        // 雪地模式：提前换挡，限制扭矩
        shift_points_[DrivingMode::SNOW] = { 1800.0, 1200.0 };
        // 手动模式：不自动换挡
        shift_points_[DrivingMode::MANUAL] = { 6000.0, 1000.0 };
    }
    
    void on_engine_rpm_changed(double rpm) {
        if (!is_automatic_ || current_mode_ == DrivingMode::MANUAL) {
            return;
        }
        
        const ShiftPoint& points = shift_points_[current_mode_];
        
        // 自动换挡逻辑
        if (current_gear_ < max_gear_ && rpm > points.upshift_rpm) {
            // 请求升挡
            if (event_bus_) {
                event_bus_->publish({
                    VehicleEventType::GEAR_SHIFT_REQUESTED,
                    { .gear_number = current_gear_ + 1 },
                    transmission_id_,
                    std::chrono::system_clock::now()
                });
            }
        } else if (current_gear_ > 1 && rpm < points.downshift_rpm) {
            // 请求降挡
            if (event_bus_) {
                event_bus_->publish({
                    VehicleEventType::GEAR_SHIFT_REQUESTED,
                    { .gear_number = current_gear_ - 1 },
                    transmission_id_,
                    std::chrono::system_clock::now()
                });
            }
        }
    }
    
    void on_vehicle_speed_changed(double speed) {
        current_speed_ = speed;
        
        // 根据车速限制档位（防止低速高档或高速低档）
        if (is_automatic_) {
            double min_speed_for_gear = (current_gear_ - 1) * 20.0; // 简化计算
            double max_speed_for_gear = current_gear_ * 40.0;
            
            if (speed < min_speed_for_gear && current_gear_ > 1) {
                // 车速过低，强制降挡
                shift_gear(current_gear_ - 1);
            } else if (speed > max_speed_for_gear && current_gear_ < max_gear_) {
                // 车速过高，强制升挡
                shift_gear(current_gear_ + 1);
            }
        }
    }
    
    void on_driving_mode_changed(DrivingMode mode) {
        current_mode_ = mode;
        is_automatic_ = (mode != DrivingMode::MANUAL);
    }
};

// ==================== 刹车系统 ====================
class BrakeSystem {
private:
    VehicleEventBus* event_bus_;
    std::string brake_id_;
    
    double brake_pressure_;      // 刹车压力 0-100%
    bool abs_active_;            // ABS是否激活
    double brake_temperature_;   // 刹车温度
    
public:
    BrakeSystem(VehicleEventBus* bus, const std::string& id)
        : event_bus_(bus), brake_id_(id),
          brake_pressure_(0.0), abs_active_(false),
          brake_temperature_(25.0) {
        
        // 刹车系统主动发布事件，通常由外部输入控制
    }
    
    void apply_brake(double pressure) {
        if (pressure < 0.0) pressure = 0.0;
        if (pressure > 100.0) pressure = 100.0;
        
        brake_pressure_ = pressure;
        
        if (event_bus_) {
            event_bus_->publish({
                VehicleEventType::BRAKE_PEDAL_PRESSED,
                { .brake_pressure = pressure },
                brake_id_,
                std::chrono::system_clock::now()
            });
            
            // 如果急刹车，发送警报
            if (pressure > 80.0) {
                event_bus_->publish({
                    VehicleEventType::SYSTEM_ALERT,
                    { .gear_number = 1 }, // 使用gear_number传递警报级别
                    brake_id_,
                    std::chrono::system_clock::now()
                });
            }
        }
    }
    
    double get_brake_pressure() const { return brake_pressure_; }
};

// ==================== 车速传感器 ====================
class SpeedSensor {
private:
    VehicleEventBus* event_bus_;
    std::string sensor_id_;
    
    double current_speed_;
    double wheel_circumference_;
    int current_gear_;
    double current_rpm_;
    
    std::thread sensor_thread_;
    std::atomic<bool> running_{true};
    std::mutex sensor_mutex_;
    
public:
    SpeedSensor(VehicleEventBus* bus, const std::string& id)
        : event_bus_(bus), sensor_id_(id),
          current_speed_(0.0), wheel_circumference_(2.0 * 3.14159 * 0.3),
          current_gear_(1), current_rpm_(800.0) {
        
        // 订阅相关事件
        if (event_bus_) {
            event_bus_->subscribe(VehicleEventType::ENGINE_RPM_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_engine_rpm_changed(event.data.engine_rpm);
                });
            
            event_bus_->subscribe(VehicleEventType::GEAR_SHIFT_COMPLETED,
                [this](const VehicleEvent& event) {
                    this->on_gear_changed(event.data.gear_number);
                });
        }
        
        sensor_thread_ = std::thread([this] { this->sensor_loop(); });
    }
    
    ~SpeedSensor() {
        running_ = false;
        if (sensor_thread_.joinable()) {
            sensor_thread_.join();
        }
    }
    
private:
    void on_engine_rpm_changed(double rpm) {
        std::lock_guard<std::mutex> lock(sensor_mutex_);
        current_rpm_ = rpm;
    }
    
    void on_gear_changed(int gear) {
        std::lock_guard<std::mutex> lock(sensor_mutex_);
        current_gear_ = gear;
    }
    
    void sensor_loop() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::normal_distribution<> noise_dist(0.0, 0.5);
        
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 5Hz
            
            double rpm, gear;
            {
                std::lock_guard<std::mutex> lock(sensor_mutex_);
                rpm = current_rpm_;
                gear = current_gear_;
            }
            
            // 计算车速：简化公式
            // 车速(km/h) = (引擎转速 * 车轮周长 * 60) / (传动比 * 1000)
            // 传动比随档位变化
            double gear_ratio = 3.0 / gear; // 简化
            double speed_kmh = (rpm * wheel_circumference_ * 60.0) / 
                               (gear_ratio * 1000.0);
            
            // 添加噪声
            speed_kmh += noise_dist(rng);
            if (speed_kmh < 0.0) speed_kmh = 0.0;
            
            current_speed_ = speed_kmh;
            
            // 发布车速变化事件
            if (event_bus_ && running_) {
                event_bus_->publish({
                    VehicleEventType::VEHICLE_SPEED_CHANGED,
                    { .vehicle_speed = speed_kmh },
                    sensor_id_,
                    std::chrono::system_clock::now()
                });
            }
        }
    }
};

// ==================== 驾驶模式管理器 ====================
class DrivingModeManager {
private:
    VehicleEventBus* event_bus_;
    std::string manager_id_;
    
    DrivingMode current_mode_;
    std::map<DrivingMode, std::string> mode_names_;
    
public:
    DrivingModeManager(VehicleEventBus* bus, const std::string& id)
        : event_bus_(bus), manager_id_(id),
          current_mode_(DrivingMode::NORMAL) {
        
        // 初始化模式名称
        mode_names_[DrivingMode::ECONOMY] = "经济模式";
        mode_names_[DrivingMode::NORMAL] = "普通模式";
        mode_names_[DrivingMode::SPORT] = "运动模式";
        mode_names_[DrivingMode::SNOW] = "雪地模式";
        mode_names_[DrivingMode::MANUAL] = "手动模式";
    }
    
    void set_driving_mode(DrivingMode mode) {
        if (mode != current_mode_) {
            current_mode_ = mode;
            
            if (event_bus_) {
                event_bus_->publish({
                    VehicleEventType::DRIVING_MODE_CHANGED,
                    { .driving_mode = static_cast<int>(mode) },
                    manager_id_,
                    std::chrono::system_clock::now()
                });
            }
        }
    }
    
    DrivingMode get_current_mode() const { return current_mode_; }
    std::string get_mode_name(DrivingMode mode) const {
        auto it = mode_names_.find(mode);
        return it != mode_names_.end() ? it->second : "未知模式";
    }
    
    DrivingMode cycle_next_mode() {
        int current = static_cast<int>(current_mode_);
        int next = (current + 1) % 5; // 5种模式
        DrivingMode next_mode = static_cast<DrivingMode>(next);
        set_driving_mode(next_mode);
        return next_mode;
    }
};

// ==================== 仪表盘 ====================
class Dashboard {
private:
    VehicleEventBus* event_bus_;
    std::string dashboard_id_;
    
    // 显示数据
    double vehicle_speed_;
    double engine_rpm_;
    int current_gear_;
    DrivingMode driving_mode_;
    double throttle_position_;
    double brake_pressure_;
    
    mutable std::mutex display_mutex_;
    
public:
    Dashboard(VehicleEventBus* bus, const std::string& id)
        : event_bus_(bus), dashboard_id_(id),
          vehicle_speed_(0.0), engine_rpm_(0.0),
          current_gear_(1), driving_mode_(DrivingMode::NORMAL),
          throttle_position_(0.0), brake_pressure_(0.0) {
        
        if (event_bus_) {
            // 订阅所有显示相关事件
            event_bus_->subscribe(VehicleEventType::VEHICLE_SPEED_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_vehicle_speed_changed(event.data.vehicle_speed);
                });
            
            event_bus_->subscribe(VehicleEventType::ENGINE_RPM_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_engine_rpm_changed(event.data.engine_rpm);
                });
            
            event_bus_->subscribe(VehicleEventType::GEAR_SHIFT_COMPLETED,
                [this](const VehicleEvent& event) {
                    this->on_gear_changed(event.data.gear_number);
                });
            
            event_bus_->subscribe(VehicleEventType::DRIVING_MODE_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_driving_mode_changed(static_cast<DrivingMode>(event.data.driving_mode));
                });
            
            event_bus_->subscribe(VehicleEventType::THROTTLE_POSITION_CHANGED,
                [this](const VehicleEvent& event) {
                    this->on_throttle_changed(event.data.throttle_position);
                });
            
            event_bus_->subscribe(VehicleEventType::BRAKE_PEDAL_PRESSED,
                [this](const VehicleEvent& event) {
                    this->on_brake_changed(event.data.brake_pressure);
                });
        }
    }
    
    void display() const {
        std::lock_guard<std::mutex> lock(display_mutex_);
        
        std::system("clear");
        std::cout << "=== 智能驾驶控制系统 ===" << std::endl;
        std::cout << std::fixed << std::setprecision(1);
        
        // 车速表（模拟圆形仪表）
        std::cout << "车速: " << vehicle_speed_ << " km/h" << std::endl;
        std::cout << "[";
        int bars = static_cast<int>(vehicle_speed_ / 5);
        for (int i = 0; i < 20; i++) {
            std::cout << (i < bars ? "=" : " ");
        }
        std::cout << "]" << std::endl;
        
        // 转速表
        std::cout << "转速: " << engine_rpm_ << " RPM" << std::endl;
        std::cout << "[";
        bars = static_cast<int>(engine_rpm_ / 300);
        for (int i = 0; i < 20; i++) {
            std::cout << (i < bars ? "=" : " ");
        }
        std::cout << "]" << std::endl;
        
        // 其他信息
        std::cout << "档位: " << (current_gear_ == 0 ? "N" : 
                                 current_gear_ == -1 ? "R" : 
                                 std::to_string(current_gear_)) << std::endl;
        std::cout << "驾驶模式: ";
        switch (driving_mode_) {
            case DrivingMode::ECONOMY: std::cout << "经济模式"; break;
            case DrivingMode::NORMAL: std::cout << "普通模式"; break;
            case DrivingMode::SPORT: std::cout << "运动模式"; break;
            case DrivingMode::SNOW: std::cout << "雪地模式"; break;
            case DrivingMode::MANUAL: std::cout << "手动模式"; break;
        }
        std::cout << std::endl;
        
        std::cout << "油门: " << throttle_position_ << "%" << std::endl;
        std::cout << "刹车: " << brake_pressure_ << "%" << std::endl;
        
        std::cout << "========================" << std::endl;
        std::cout << "操作选项:" << std::endl;
        std::cout << "W: 加速  S: 减速  A: 左转  D: 右转" << std::endl;
        std::cout << "B: 刹车  M: 切换驾驶模式  G: 手动换挡" << std::endl;
        std::cout << "Q: 退出系统" << std::endl;
        std::cout << "请选择操作: ";
    }
    
private:
    void on_vehicle_speed_changed(double speed) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        vehicle_speed_ = speed;
    }
    
    void on_engine_rpm_changed(double rpm) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        engine_rpm_ = rpm;
    }
    
    void on_gear_changed(int gear) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        current_gear_ = gear;
    }
    
    void on_driving_mode_changed(DrivingMode mode) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        driving_mode_ = mode;
    }
    
    void on_throttle_changed(double throttle) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        throttle_position_ = throttle;
    }
    
    void on_brake_changed(double brake) {
        std::lock_guard<std::mutex> lock(display_mutex_);
        brake_pressure_ = brake;
    }
};

// ==================== 主控制系统 ====================
class VehicleControlSystem {
private:
    std::unique_ptr<VehicleEventBus> event_bus_;
    std::unique_ptr<EngineController> engine_controller_;
    std::unique_ptr<TransmissionController> transmission_controller_;
    std::unique_ptr<BrakeSystem> brake_system_;
    std::unique_ptr<SpeedSensor> speed_sensor_;
    std::unique_ptr<DrivingModeManager> driving_mode_manager_;
    std::unique_ptr<Dashboard> dashboard_;
    
    std::thread event_thread_;
    std::atomic<bool> running_{true};
    
    // 用户输入状态
    double current_throttle_;
    double current_brake_;
    
public:
    VehicleControlSystem() 
        : current_throttle_(0.0), current_brake_(0.0) {
        
        // 创建事件总线
        event_bus_ = std::make_unique<VehicleEventBus>();
        
        // 创建组件
        engine_controller_ = std::make_unique<EngineController>(event_bus_.get(), "Engine1");
        transmission_controller_ = std::make_unique<TransmissionController>(event_bus_.get(), "Transmission1");
        brake_system_ = std::make_unique<BrakeSystem>(event_bus_.get(), "Brake1");
        speed_sensor_ = std::make_unique<SpeedSensor>(event_bus_.get(), "SpeedSensor1");
        driving_mode_manager_ = std::make_unique<DrivingModeManager>(event_bus_.get(), "ModeManager1");
        dashboard_ = std::make_unique<Dashboard>(event_bus_.get(), "Dashboard1");
        
        // 启动事件处理线程
        event_thread_ = std::thread([this] { 
            this->event_bus_->process_events(); 
        });
        
        // 设置初始模式
        driving_mode_manager_->set_driving_mode(DrivingMode::NORMAL);
    }
    
    ~VehicleControlSystem() {
        running_ = false;
        event_bus_->stop();
        if (event_thread_.joinable()) {
            event_thread_.join();
        }
    }
    
    void run() {
        std::cout << "智能驾驶控制系统启动..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        while (running_) {
            dashboard_->display();
            
            char input;
            std::cin >> input;
            
            handle_input(input);
            
            // 模拟物理衰减
            simulate_decay();
        }
    }
    
private:
    void handle_input(char input) {
        switch (std::tolower(input)) {
            case 'w': // 加速
                current_throttle_ += 10.0;
                if (current_throttle_ > 100.0) current_throttle_ = 100.0;
                engine_controller_->set_throttle(current_throttle_);
                break;
                
            case 's': // 减速
                current_throttle_ -= 10.0;
                if (current_throttle_ < 0.0) current_throttle_ = 0.0;
                engine_controller_->set_throttle(current_throttle_);
                break;
                
            case 'b': // 刹车
                current_brake_ += 20.0;
                if (current_brake_ > 100.0) current_brake_ = 100.0;
                brake_system_->apply_brake(current_brake_);
                break;
                
            case 'm': // 切换驾驶模式
                {
                    DrivingMode new_mode = driving_mode_manager_->cycle_next_mode();
                    std::cout << "切换到: ";
                    switch (new_mode) {
                        case DrivingMode::ECONOMY: std::cout << "经济模式"; break;
                        case DrivingMode::NORMAL: std::cout << "普通模式"; break;
                        case DrivingMode::SPORT: std::cout << "运动模式"; break;
                        case DrivingMode::SNOW: std::cout << "雪地模式"; break;
                        case DrivingMode::MANUAL: std::cout << "手动模式"; break;
                    }
                    std::cout << std::endl;
                }
                break;
                
            case 'g': // 手动换挡
                if (driving_mode_manager_->get_current_mode() == DrivingMode::MANUAL) {
                    std::cout << "输入档位 (1-6, 0=N, -1=R): ";
                    int gear;
                    std::cin >> gear;
                    transmission_controller_->shift_gear(gear);
                } else {
                    std::cout << "当前不是手动模式，无法手动换挡" << std::endl;
                }
                break;
                
            case 'q': // 退出
                running_ = false;
                std::cout << "系统正在关闭..." << std::endl;
                break;
                
            default:
                std::cout << "无效输入，请重试" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
        }
    }
    
    void simulate_decay() {
        // 模拟油门和刹车的自然衰减
        if (current_throttle_ > 0.0) {
            current_throttle_ *= 0.95;
            if (current_throttle_ < 1.0) current_throttle_ = 0.0;
            engine_controller_->set_throttle(current_throttle_);
        }
        
        if (current_brake_ > 0.0) {
            current_brake_ *= 0.8;
            if (current_brake_ < 1.0) current_brake_ = 0.0;
            brake_system_->apply_brake(current_brake_);
        }
    }
};

// ==================== 主函数 ====================
int main() {
    try {
        VehicleControlSystem system;
        system.run();
        std::cout << "系统已关闭" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "系统错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}