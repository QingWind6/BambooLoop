#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>

// App 生命周期状态
enum class AppState {
    SETUP,      // 等待初始化
    RUNNING,    // 正在运行
    DYING,      // 等待销毁
    DEAD        // 已销毁
};

class Scheduler; // 前向声明

/**
 * @brief App 基类
 */
class AppBase {
    friend class Scheduler;
private:
    AppState _state = AppState::SETUP;

    void _internal_update() {
        switch (_state) {
            case AppState::SETUP:
                onSetup();
                _state = AppState::RUNNING;
                break; 
            case AppState::RUNNING:
                onRunning();
                break;
            case AppState::DYING:
                onDestroy();
                _state = AppState::DEAD;
                break;
            default: break;
        }
    }

public:
    virtual ~AppBase() = default;

    virtual void onSetup() {}
    virtual void onRunning() {}
    virtual void onDestroy() {}

    // 请求销毁当前 App
    void kill() { _state = AppState::DYING; }
    // 检查是否已死亡
    bool isDead() const { return _state == AppState::DEAD; }
};

/**
 * @brief 调度器
 */
class Scheduler {
private:
    // 独占管理 App 内存
    std::vector<std::unique_ptr<AppBase>> _apps;
    std::vector<std::unique_ptr<AppBase>> _new_apps; 

public:
    /**
     * @brief 安装 App (移交所有权)
     */
    void install(std::unique_ptr<AppBase> app) {
        if (app) _new_apps.push_back(std::move(app));
    }

    /**
     * @brief 辅助安装函数 (直接构造)
     * 用法: scheduler.install<MyApp>(arg1, arg2);
     */
    template <typename T, typename... Args>
    T* install(Args&&... args) {
        auto app = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = app.get(); // 获取裸指针返回给用户以备不时之需
        _new_apps.push_back(std::move(app));
        return ptr;
    }

    /**
     * @brief 主循环 update
     */
    void update() {
        // 1. 将新 App 移入主列表
        if (!_new_apps.empty()) {
            std::move(_new_apps.begin(), _new_apps.end(), std::back_inserter(_apps));
            _new_apps.clear();
        }

        // 2. 遍历列表
        for (auto it = _apps.begin(); it != _apps.end(); ) {
            auto& app = *it;
            
            // 使用 friend 权限访问 private 的 _internal_update
            app->_internal_update();

            // 3. 清理已死亡的 App
            if (app->isDead()) {
                // erase 会销毁 unique_ptr，从而自动调用 delete app
                it = _apps.erase(it); 
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief 获取指定类型的 App 实例 (Observer 模式)
     * * @tparam T 目标 App 类型
     * @return T* 目标 App 的裸指针，如果未找到则返回 nullptr
     */
    template <typename T>
    T* getApp() {
        // 先在主列表中查找
        for (const auto& app : _apps) {
            // dynamic_cast 需要目标类有虚函数 (App 已经有虚析构了)
            // 且编译器开启 RTTI (ESP32 Arduino 默认开启)
            T* ptr = dynamic_cast<T*>(app.get());
            if (ptr && !ptr->isDead()) return ptr;
        }
        // 也要在新添加列表中查找 (防止刚 install 还没 update 就想获取的情况)
        for (const auto& app : _new_apps) {
            T* ptr = dynamic_cast<T*>(app.get());
            if (ptr && !ptr->isDead()) return ptr;
        }
        return nullptr;
    }
    
    // 获取当前 App 数量
    size_t getAppCount() const { return _apps.size() + _new_apps.size(); }
};