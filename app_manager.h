// /**
//  * @file app_manager.h
//  * @brief 简单的 App 调度器
//  */
// #pragma once
// #include "app_base.h"
// #include <vector>
// #include <stack>

// class AppManager {
// private:
//     std::stack<AppBase*> _app_stack;
//     AppBase* _current_app = nullptr;

// public:
//     /* 安装/启动一个 App (Push) */
//     void startApp(AppBase* app) {
//         if (_current_app) {
//             _current_app->onPause();
//         }

//         _app_stack.push(app);
//         _current_app = app;

//         _current_app->onSetup();
//         _current_app->onResume();
//     }

//     /* 关闭当前 App，返回上一级 (Pop) */
//     void closeApp() {
//         if (_app_stack.empty()) return;

//         AppBase* app = _app_stack.top();
//         app->onPause();
//         app->onDestroy();
        
//         delete app; 
        
//         _app_stack.pop();

//         if (!_app_stack.empty()) {
//             _current_app = _app_stack.top();
//             _current_app->onResume();
//         } else {
//             _current_app = nullptr;
//         }
//     }

//     /* 在主循环中调用 */
//     void update() {
//         if (_current_app) {
//             _current_app->onRunning();
//         }
//     }
// };