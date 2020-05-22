#ifndef DEAR_CORE_APPLICATION_H_
#define DEAR_CORE_APPLICATION_H_

#include <vector>
#include <memory>
#include <functional>

#include "sokol_app.h"
#include "sokol_gfx.h"

#include "applet.h"

namespace dear::core {

class application {
public:
    // コンストラクタ
    application();

    // デストラクタ
    virtual ~application();

    // セットアップ
    void setup(int argc, char** argv, sapp_desc &desc);

    // シャットダウン
    void shutdown();

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

    // 初期化関数型
    using init_fn = std::function<void()>;

    // フレーム経過関数型
    using frame_fn = std::function<void(double)>;

    // クリーンアップ関数型
    using cleanup_fn = std::function<void()>;

    // イベント関数型
    using event_fn = std::function<bool(const sapp_event *)>;

    // エラー関数型
    using fail_fn = std::function<void(const char *)>;

    // 初期化コールバックの追加
    void add_init_callback(init_fn fn) { _init_callbacks.emplace_back(fn); }

    // フレーム経過コールバックの追加
    void add_frame_callback(frame_fn fn) { _frame_callbacks.emplace_back(fn); }

    // クリーンアップコールバックの追加
    void add_cleanup_callback(cleanup_fn fn) { _cleanup_callbacks.emplace_back(fn); }

    // イベントコールバックの追加
    void add_event_callback(event_fn fn) { _event_callbacks.emplace_back(fn); }

    // エラーコールバックの追加
    void add_fail_callback(fail_fn fn) { _fail_callbacks.emplace_back(fn); }

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

    // アプレットを返す
    auto &get_applets() const { return _applets; }

private:
    // 前回の時間
    uint64_t _last_time = 0;

    // 描画アクション
    sg_pass_action _pass_action;

    // アプレットリスト
    std::vector<std::unique_ptr<applet>> _applets;

    // コールバックリスト
    std::vector<init_fn> _init_callbacks;
    std::vector<frame_fn> _frame_callbacks;
    std::vector<cleanup_fn> _cleanup_callbacks;
    std::vector<event_fn> _event_callbacks;
    std::vector<fail_fn> _fail_callbacks;
};

} // namespace dear::core

#define DEAR_MAIN(app) \
sapp_desc sokol_main(int argc, char* argv[]) { \
    static app _app; \
    sapp_desc desc = {}; \
    _app.setup(argc, argv, desc); \
    return desc; \
}

#endif // DEAR_CORE_APPLICATION_H_
