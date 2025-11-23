#include <bits/stdc++.h>

class Observer;
class Subject {
public:
    Subject() : m_nState(0) {}

    virtual ~Subject() = default;

    virtual void Attach(const std::shared_ptr<Observer> pObserver) = 0;

    virtual void Detach(const std::shared_ptr<Observer> pObserver) = 0;

    virtual void Notify() = 0;

    virtual int GetState() { return m_nState; }

    void SetState(int state) {
        std::cout << "Subject updated !" << std::endl;
        m_nState = state;
    }

protected:
    std::list<std::shared_ptr<Observer>> m_pObserver_list;
    int m_nState;
};

class Observer {
public:
    virtual ~Observer() = default;

    Observer(const std::shared_ptr<Subject> pSubject, const std::string &name = "unknown")
        : m_pSubject(pSubject), m_strName(name) {}


    virtual void Update() = 0;

    virtual const std::string &name() { return m_strName; }


protected:
    std::shared_ptr<Subject> m_pSubject;
    std::string m_strName;
};

//具体被观察者
class ConcreteSubject : public Subject {
public:
    void Attach(const std::shared_ptr<Observer> pObserver) override {
        auto iter = std::find(m_pObserver_list.begin(), m_pObserver_list.end(), pObserver);
        if (iter == m_pObserver_list.end()) {
            std::cout << "Attach observer" << pObserver->name() << std::endl;
            m_pObserver_list.emplace_back(pObserver);
        }

    }

    void Detach(const std::shared_ptr<Observer> pObserver) override {
        std::cout << "Detach observer" << pObserver->name() << std::endl;
        m_pObserver_list.remove(pObserver);
    }

    //循环通知所有观察者
    void Notify() override {
        auto it = m_pObserver_list.begin();
        while (it != m_pObserver_list.end()) {
            (*it++)->Update();
        }
    }
};

//具体观察者1
class Observer1 : public Observer {
public:
    Observer1(const std::shared_ptr<Subject> pSubject, const std::string &name = "unknown")
            : Observer(pSubject, name) {}

    void Update() override {
        std::cout << "Observer1_" << m_strName << " get the update.New state is: "
                  << m_pSubject->GetState() << std::endl;
    }
};

//具体观察者2
class Observer2 : public Observer {
public:
    Observer2(const std::shared_ptr<Subject> pSubject, const std::string &name = "unknown")
            : Observer(pSubject, name) {}

    void Update() override {
        std::cout << "Observer2_" << m_strName << " get the update.New state is: "
                  << m_pSubject->GetState() << std::endl;
    }
};

int main() {

    std::shared_ptr<Subject> pSubject = std::make_shared<ConcreteSubject>();// 创建被观察者

    // 创建观察者
    std::shared_ptr<Observer> pObserver1_1 = std::make_shared<Observer1>(pSubject, "1");
    std::shared_ptr<Observer> pObserver1_2 = std::make_shared<Observer1>(pSubject, "2");
    std::shared_ptr<Observer> pObserver1_3 = std::make_shared<Observer1>(pSubject, "3");

    std::shared_ptr<Observer> pObserver2_4 = std::make_shared<Observer2>(pSubject, "4");
    std::shared_ptr<Observer> pObserver2_5 = std::make_shared<Observer2>(pSubject, "5");
    std::shared_ptr<Observer> pObserver2_6 = std::make_shared<Observer2>(pSubject, "6");

    // 注册观察者
    pSubject->Attach(pObserver1_1);
    pSubject->Attach(pObserver1_2);
    pSubject->Attach(pObserver1_3);
    pSubject->Attach(pObserver2_4);
    pSubject->Attach(pObserver2_5);
    pSubject->Attach(pObserver2_6);

    pSubject->SetState(2);// 改变状态
    pSubject->Notify();

    std::cout << std::string(50, '-') << std::endl;

    // 注销观察者
    pSubject->Detach(pObserver1_1);
    pSubject->Detach(pObserver1_2);

    pSubject->SetState(3);
    pSubject->Notify();

    return 0;
    //运行结果如下：
    //Attach observer1
    //Attach observer2
    //Attach observer3
    //Attach observer4
    //Attach observer5
    //Attach observer6
    //Subject updated !
    //Observer1_1 get the update.New state is: 2
    //Observer1_2 get the update.New state is: 2
    //Observer1_3 get the update.New state is: 2
    //Observer2_4 get the update.New state is: 2
    //Observer2_5 get the update.New state is: 2
    //Observer2_6 get the update.New state is: 2
    //--------------------------------------------------
    //Detach observer1
    //Detach observer2
    //Subject updated !
    //Observer1_3 get the update.New state is: 3
    //Observer2_4 get the update.New state is: 3
    //Observer2_5 get the update.New state is: 3
    //Observer2_6 get the update.New state is: 3

}