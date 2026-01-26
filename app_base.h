/**
 * @file app_base.h
 * @brief 类似 iOS UIViewController 的基类
 */
#pragma once
#include <string>

enum class AppState {
    CREATED,
    RUNNING,
    BACKGROUND,
    DESTROYED
};

class AppBase {
protected:
    std::string _app_name;
    void* _user_data = nullptr;

public:
    AppBase(const std::string& name) : _app_name(name) {}
    virtual ~AppBase() {}


    virtual void onSetup() {}

    virtual void onResume() {}

    virtual void onRunning() {}

    virtual void onPause() {}

    virtual void onDestroy() {}

    std::string getName() { return _app_name; }
};