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

    // 初期設定
    virtual void configure(application *app) {}
};

} // namespace dear::core

#endif // DEAR_CORE_APPLET_H_
