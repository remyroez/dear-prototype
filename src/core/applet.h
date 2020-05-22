#ifndef DEAR_CORE_APPLET_H_
#define DEAR_CORE_APPLET_H_

#include "sokol_app.h"

namespace dear::core {

class applet {
public:
    // コンストラクタ
    applet() {}

    // デストラクタ
    virtual ~applet() {}

    // 名前
    virtual const char *name() = 0;

    // 初期化
    virtual void init() {}

    // フレーム経過
    virtual void frame(double delta_time) {}

    // クリーンアップ
    virtual void cleanup() {}

    // イベント
    virtual bool event(const sapp_event *ev) { return false; }
};

} // namespace dear::core

#endif // DEAR_CORE_APPLET_H_
