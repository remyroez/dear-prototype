#ifndef DEAR_CORE_APPLICATION_H_
#define DEAR_CORE_APPLICATION_H_

#include <vector>
#include <memory>

#include "sokol_app.h"
#include "sokol_gfx.h"

#include "applet.h"

namespace dear::core {

class application {
public:
    // コンストラクタ
    application(int argc, char** argv);

    // デストラクタ
    virtual ~application();

    // 初期設定コールバック
    void configure_cb(sapp_desc &desc);

    // 初期化コールバック
    void init_cb();

    // フレーム経過コールバック
    void frame_cb();

    // クリーンアップコールバック
    void cleanup_cb();

    // イベントコールバック
    void event_cb(const sapp_event *ev);

    // エラーコールバック
    void fail_cb(const char *message);

    // 背景色の設定
    void set_background_color(float r, float g, float b, float a = 1.f) {
        _pass_action.colors[0].val[0] = r;
        _pass_action.colors[0].val[1] = g;
        _pass_action.colors[0].val[2] = b;
        _pass_action.colors[0].val[3] = a;
    }

    // アプレット登録
    template <class T, class ...Args>
    inline void make_applet(Args &&...args) {
        _applets.emplace_back(std::make_unique<T>(std::forward(args)...));
    }

protected:
    // 初期設定
    virtual void configure(sapp_desc &desc) {}

    // 初期化
    virtual void init() {}

    // フレーム経過
    virtual void frame(double delta_time) {}

    // クリーンアップ
    virtual void cleanup() {}

    // イベント
    virtual bool event(const sapp_event *ev) { return false; }

    // エラー
    virtual void fail(const char *message) {}

private:
    // 前回の時間
    uint64_t _last_time = 0;

    // 描画アクション
    sg_pass_action _pass_action;

    // アプレットリスト
    std::vector<std::unique_ptr<applet>> _applets;
};

} // namespace dear::core

#define DEAR_MAIN(app) \
sapp_desc sokol_main(int argc, char* argv[]) { \
    static app _app(argc, argv); \
    sapp_desc desc = {}; \
    _app.configure_cb(desc); \
    return desc; \
}

#endif // DEAR_CORE_APPLICATION_H_
