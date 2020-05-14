#ifndef DEAR_CORE_APPLICATION_H_
#define DEAR_CORE_APPLICATION_H_

#include "sokol_app.h"
#include "sokol_gfx.h"

namespace dear::core {

class application {
public:
    // コンストラクタ
    application(int argc, char** argv);

    // デストラクタ
    virtual ~application();

    // 初期設定
    virtual void configure(sapp_desc &desc);

    // 初期化
    virtual void init();

    // フレーム経過
    virtual void frame();

    // クリーンアップ
    virtual void cleanup();

    // イベント
    virtual void event(const sapp_event *ev);

    // エラー
    virtual void fail(const char *message);

private:
    uint64_t _last_time = 0;
    sg_pass_action _pass_action;
};

} // namespace dear::core

#define DEAR_MAIN(app) \
sapp_desc sokol_main(int argc, char* argv[]) { \
    static app _app(argc, argv); \
    sapp_desc desc = {}; \
    _app.configure(desc); \
    return desc; \
}

#endif // DEAR_CORE_APPLICATION_H_
