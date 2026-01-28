#pragma once
#include <vector>
#include <memory>
#include <algorithm>

// 1. 定义应用的状态
enum class AppState {
    SETUP,      // 刚创建，等待初始化
    RUNNING,    // 正在运行
    DYING,      // 等待销毁
    DEAD        // 已销毁
};

// 2. App 基类
class App {
    friend class Scheduler;
private:
    AppState _state = AppState::SETUP;

    // 内部生命周期管理
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
    virtual ~App() = default;

    virtual void onSetup() {}
    virtual void onRunning() {}
    virtual void onDestroy() {}

    // --- 控制接口 ---
    void kill() { _state = AppState::DYING; }
    bool isDead() { return _state == AppState::DEAD; }
};

// 3. 调度器
class Scheduler {
private:
    std::vector<std::shared_ptr<App>> _apps;
    std::vector<std::shared_ptr<App>> _new_apps; // 缓冲队列，防止遍历时插入导致迭代器失效

public:
    void install(std::shared_ptr<App> app) {
        if (app) _new_apps.push_back(app);
    }

    void update() {
        // 1. 引入新 App
        if (!_new_apps.empty()) {
            _apps.insert(_apps.end(), _new_apps.begin(), _new_apps.end());
            _new_apps.clear();
        }

        // 2. 遍历并更新所有 App
        for (auto it = _apps.begin(); it != _apps.end(); ) {
            auto& app = *it;
            
            // 执行 App 内部逻辑
            // 为了安全还是设置为private,作为友元类调用
            ((App*)app.get())->_internal_update();

            // 3. 清理已死亡的 App
            if (app->isDead()) {
                it = _apps.erase(it); // 从列表中移除，智能指针会自动 delete 对象
            } else {
                ++it;
            }
        }
    }
};