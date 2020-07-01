#ifndef DEAR_CORE_APPLET_H_
#define DEAR_CORE_APPLET_H_

#include "sokol_app.h"

namespace dear::core {

class application;

class applet {
public:
    // デストラクタ
    virtual ~applet() {}

    // 名前
    virtual const char *name() = 0;

    // インストール
    virtual void install(application *app) {}

    // アンインストール
    virtual void uninstall(application *app) {}

    // 設定
    virtual void settings() {}
};

} // namespace dear::core

#endif // DEAR_CORE_APPLET_H_
