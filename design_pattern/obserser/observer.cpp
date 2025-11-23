#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>
#include <random>
#include <algorithm>


//前向声明
class SmartCar;

class CarObserver {
public:
    virtual ~CarObserver() = default;
    virtual void update(SmartCar* car) = 0;
    virtual std::string get_name() const = 0;
};


class SmartCar{
private:
    float cur_speed_;               // 车辆速度
    float motor_speed_;             // 电机速度
    float car_angle_;               // 转向角度
    std::string camera_frame_;      //摄像头帧数据

    std::vector<CarObserver*> observers_;

public:
    SmartCar() : motor_speed_(0.0f), car_angle_(0.0f), cur_speed_(0.0f), camera_frame_("No frame") {}
    virtual ~SmartCar() = default;

    void attach(CarObserver* observer){
        observers_.push_back(observer);
        std::cout << observer->get_name() << "已注册为观察者\n" ;
    }

    void detach(CarObserver* observer){
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
        std::cout << observer->get_name() << "已移除观察者\n" ;
    }

    void notify(){
        for(auto observer : observers_){
            observer->update(this);
        }
    }

    void set_steer_angle(float angle) {
        car_angle_ = angle;
        std::cout << "转向角度设置为: " << car_angle_ << " 弧度\n";
        notify();
    }

    void set_motor_speed(float speed) {
        motor_speed_ = speed;
        cur_speed_ = speed * 0.1f;
        std::cout << "电机转速设置为: " << motor_speed_ << " RPM, 当前速度: " << cur_speed_ << " km/h\n";
        notify();
    }


    void camera_capture_frame() {
        // 模拟生成不同的帧数据
        static int frameCount = 0;
        camera_frame_ = "Frame_" + std::to_string(++frameCount);
        std::cout << "摄像头捕获: " << camera_frame_ << "\n";
        notify();
    }

    float get_cur_speed() {return cur_speed_;}
    float get_car_speed() {return motor_speed_;}
    float get_car_angle() {return car_angle_;}
    std::string get_camera_frame() const {return camera_frame_;}

};

// 转向状态显示器
class SteerMonitor : public CarObserver {
public:
    SteerMonitor() = default;

    void update(SmartCar* car) override {
        float angle = car->get_car_angle();
        float degree = angle * 180.0 / M_PI;
        std::cout << "舵机当前角度 ： " << degree << "°\n";
    }

    std::string get_name() const override {
        return "转向显示器";
    }
};

// 速度显示器
class SpeedMonitor : public CarObserver {
public:
    SpeedMonitor() = default;

    void update(SmartCar* car) override {
        float speed = car->get_cur_speed();
        std::cout << "车辆当前速度 ： " << speed << "km/h \n";
    }

    std::string get_name() const override {
        return "速度显示器";
    }
};

// 电机状态监控器
class MotorMonitor : public CarObserver {
public:
    MotorMonitor() = default;

    void update(SmartCar* car) override {
        float rpm = car->get_car_speed();
        std::cout << "电机监控器: 当前转速 " << rpm << " RPM\n";

        // 模拟过载保护
        if (rpm > 3000.0f) {
            std::cout << "警告: 电机转速过高!\n";
        }
    }
    std::string get_name() const override {
        return "电机监控器";
    }
};




class CameraProcessor : public CarObserver {
public:
    CameraProcessor() = default;

    void update(SmartCar* car) override {
        std::string frame = car->get_camera_frame();
            std::cout << "摄像头处理器: 处理帧 " << frame << "\n";
        
        // 模拟图像处理
        if (frame != "No frame") {
            std::cout << "进行图像分析: 车道检测、障碍物识别...\n";
        }
    }

    std::string get_name() const override {
        return "摄像头处理器";
    }
};


class AutonomousController : public CarObserver {
public:
    AutonomousController() = default;

    void update(SmartCar* car) override {
        // 基于所有传感器数据做出决策
        float speed = car->get_cur_speed();
        float steering = car->get_car_angle();
        std::string frame = car->get_camera_frame();
        
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

    std::string get_name() const override {
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
                  << "转向: " << car->get_car_angle()
                  << ", 速度: " << car->get_cur_speed()
                  << ", 电机: " << car->get_car_speed()
                  << ", 帧: " << car->get_camera_frame() << "\n";
    }
    
    std::string get_name() const override {
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
        car.set_steer_angle(steerDist(gen));
        
        // 随机改变电机速度
        car.set_motor_speed(speedDist(gen));
        
        // 捕获摄像头帧
        car.camera_capture_frame();
        
        // 短暂暂停
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main()
{
    SmartCar car;

    MotorMonitor motor_monitor;
    SteerMonitor steer_monitor;
    SpeedMonitor speed_monitor;
    CameraProcessor camera_processor;
    AutonomousController auto_controller;
    DataLogger data_logger;

    car.attach(&motor_monitor);
    car.attach(&steer_monitor);
    car.attach(&speed_monitor);
    car.attach(&camera_processor);
    car.attach(&data_logger);

    std::cout << "\n开始模拟智能车操作...\n";
    simulateCarOperation(car);
    
    std::cout << "\n移除数据记录器...\n";
    car.detach(&data_logger);
    
    std::cout << "\n注册自动驾驶控制器...\n";
    car.attach(&auto_controller);
    
    std::cout << "\n继续模拟操作...\n";
    simulateCarOperation(car);

    return 0;
}

