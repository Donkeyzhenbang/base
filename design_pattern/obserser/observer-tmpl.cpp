#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>
#include <random>
#include <algorithm>

// 前向声明
class SmartCar;

// 观察者接口
class CarObserver {
public:
    virtual ~CarObserver() = default;
    virtual void update(SmartCar* car) = 0;
    virtual std::string getName() const = 0;
};

// 智能车主题类
class SmartCar {
private:
    // 车状态数据
    float steeringAngle_;    // 转向角度（弧度）
    float motorSpeed_;       // 电机转速（RPM）
    float currentSpeed_;     // 当前速度（km/h）
    std::string cameraFrame_; // 摄像头帧数据（模拟）
    
    // 观察者列表
    std::vector<CarObserver*> observers_;
    
public:
    SmartCar() 
        : steeringAngle_(0.0f), motorSpeed_(0.0f), currentSpeed_(0.0f), cameraFrame_("No frame") {}
    
    // 注册观察者
    void attach(CarObserver* observer) {
        observers_.push_back(observer);
        std::cout << observer->getName() << " 已注册为观察者\n";
    }
    
    // 移除观察者
    void detach(CarObserver* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
        std::cout << observer->getName() << " 已移除观察者\n";
    }
    
    // 通知所有观察者
    void notify() {
        for (auto observer : observers_) {
            observer->update(this);
        }
    }
    
    // 设置转向角度
    void setSteeringAngle(float angle) {
        steeringAngle_ = angle;
        std::cout << "转向角度设置为: " << angle << " 弧度\n";
        notify();
    }
    
    // 设置电机转速
    void setMotorSpeed(float speed) {
        motorSpeed_ = speed;
        // 简化模型：速度与电机转速成正比
        currentSpeed_ = speed * 0.1f;
        std::cout << "电机转速设置为: " << speed << " RPM, 当前速度: " << currentSpeed_ << " km/h\n";
        notify();
    }
    
    // 捕获摄像头帧
    void captureCameraFrame() {
        // 模拟生成不同的帧数据
        static int frameCount = 0;
        cameraFrame_ = "Frame_" + std::to_string(++frameCount);
        std::cout << "摄像头捕获: " << cameraFrame_ << "\n";
        notify();
    }
    
    // 获取状态数据
    float getSteeringAngle() const { return steeringAngle_; }
    float getMotorSpeed() const { return motorSpeed_; }
    float getCurrentSpeed() const { return currentSpeed_; }
    std::string getCameraFrame() const { return cameraFrame_; }
};

// 转向状态显示器
class SteeringDisplay : public CarObserver {
public:
    SteeringDisplay() = default;
    
    void update(SmartCar* car) override {
        float angle = car->getSteeringAngle();
        float degrees = angle * 180.0f / M_PI; // 转换为角度
        std::cout << "转向显示器: 当前转向角度 " << degrees << "°\n";
    }
    
    std::string getName() const override {
        return "转向显示器";
    }
};

// 速度显示器
class SpeedDisplay : public CarObserver {
public:
    SpeedDisplay() = default;
    
    void update(SmartCar* car) override {
        float speed = car->getCurrentSpeed();
        std::cout << "速度显示器: 当前速度 " << speed << " km/h\n";
    }
    
    std::string getName() const override {
        return "速度显示器";
    }
};

// 电机状态监控器
class MotorMonitor : public CarObserver {
public:
    MotorMonitor() = default;
    
    void update(SmartCar* car) override {
        float rpm = car->getMotorSpeed();
        std::cout << "电机监控器: 当前转速 " << rpm << " RPM\n";
        
        // 模拟过载保护
        if (rpm > 3000.0f) {
            std::cout << "警告: 电机转速过高!\n";
        }
    }
    
    std::string getName() const override {
        return "电机监控器";
    }
};

// 摄像头处理器
class CameraProcessor : public CarObserver {
public:
    CameraProcessor() = default;
    
    void update(SmartCar* car) override {
        std::string frame = car->getCameraFrame();
        std::cout << "摄像头处理器: 处理帧 " << frame << "\n";
        
        // 模拟图像处理
        if (frame != "No frame") {
            std::cout << "进行图像分析: 车道检测、障碍物识别...\n";
        }
    }
    
    std::string getName() const override {
        return "摄像头处理器";
    }
};

// 自动驾驶控制器
class AutonomousController : public CarObserver {
public:
    AutonomousController() = default;
    
    void update(SmartCar* car) override {
        // 基于所有传感器数据做出决策
        float speed = car->getCurrentSpeed();
        float steering = car->getSteeringAngle();
        std::string frame = car->getCameraFrame();
        
        std::cout << "自动驾驶控制器: 综合决策中...\n";
        
        // 模拟决策逻辑
        if (speed > 50.0f) {
            std::cout << "决策: 减速以保持安全\n";
            // 实际中会调用 car->setMotorSpeed(...)
        }
        
        if (frame.find("obstacle") != std::string::npos) {
            std::cout << "决策: 检测到障碍物，准备避让\n";
            // 实际中会调用 car->setSteeringAngle(...)
        }
    }
    
    std::string getName() const override {
        return "自动驾驶控制器";
    }
};

// 数据记录器
class DataLogger : public CarObserver {
public:
    DataLogger() = default;
    
    void update(SmartCar* car) override {
        // 记录所有状态数据
        std::cout << "数据记录器: 记录状态 - "
                  << "转向: " << car->getSteeringAngle()
                  << ", 速度: " << car->getCurrentSpeed()
                  << ", 电机: " << car->getMotorSpeed()
                  << ", 帧: " << car->getCameraFrame() << "\n";
    }
    
    std::string getName() const override {
        return "数据记录器";
    }
};

// 模拟智能车运行
void simulateCarOperation(SmartCar& car) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> steerDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> speedDist(0.0f, 3500.0f);
    
    for (int i = 0; i < 10; ++i) {
        std::cout << "\n=== 操作 " << i + 1 << " ===\n";
        
        // 随机改变转向角度
        car.setSteeringAngle(steerDist(gen));
        
        // 随机改变电机速度
        car.setMotorSpeed(speedDist(gen));
        
        // 捕获摄像头帧
        car.captureCameraFrame();
        
        // 短暂暂停
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    // 创建智能车
    SmartCar car;
    
    // 创建各种观察者
    SteeringDisplay steeringDisplay;
    SpeedDisplay speedDisplay;
    MotorMonitor motorMonitor;
    CameraProcessor cameraProcessor;
    AutonomousController autoController;
    DataLogger dataLogger;
    
    // 注册观察者
    car.attach(&steeringDisplay);
    car.attach(&speedDisplay);
    car.attach(&motorMonitor);
    car.attach(&cameraProcessor);
    car.attach(&dataLogger);
    
    std::cout << "\n开始模拟智能车操作...\n";
    simulateCarOperation(car);
    
    std::cout << "\n移除数据记录器...\n";
    car.detach(&dataLogger);
    
    std::cout << "\n注册自动驾驶控制器...\n";
    car.attach(&autoController);
    
    std::cout << "\n继续模拟操作...\n";
    simulateCarOperation(car);
    
    return 0;
}