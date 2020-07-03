#ifndef DEAR_CORE_APPLICATION_H_
#define DEAR_CORE_APPLICATION_H_

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_imgui.h"

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

    // 終了させる
    void quit();

    // 初期設定コールバック
    void configure_cb(sapp_desc &desc);

    // 初期化コールバック
    void init_cb();

    // メインメニューコールバック
    void mainmenu_cb(double dt);

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

    // アプレットハンドル
    using applet_handle = std::weak_ptr<applet>;

    // アプレット登録
    template <class T, class ...Args>
    inline applet_handle make_applet(Args &&...args) {
        return _applets.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
    }

    // 初期化関数型
    using init_fn = std::function<void()>;

    // フレーム経過関数型
    using frame_fn = std::function<void(double)>;

    // メインメニュー関数型
    using mainmenu_fn = frame_fn;

    // 背景関数型
    using background_fn = frame_fn;

    // クリーンアップ関数型
    using cleanup_fn = std::function<void()>;

    // イベント関数型
    using event_fn = std::function<bool(const sapp_event *)>;

    // エラー関数型
    using fail_fn = std::function<void(const char *)>;

    // 初期化コールバックの追加
    void add_init_callback(init_fn fn) { _init_callbacks.emplace_back(fn); }

    // メインメニューコールバックの追加
    void add_mainmenu_callback(mainmenu_fn fn) { _mainmenu_callbacks.emplace_back(fn); }

    // 背景コールバックの追加
    void add_background_callback(background_fn fn) { _background_callbacks.emplace_back(fn); }

    // フレーム経過コールバックの追加
    void add_frame_callback(frame_fn fn) { _frame_callbacks.emplace_back(fn); }

    // クリーンアップコールバックの追加
    void add_cleanup_callback(cleanup_fn fn) { _cleanup_callbacks.emplace_back(fn); }

    // イベントコールバックの追加
    void add_event_callback(event_fn fn) { _event_callbacks.emplace_back(fn); }

    // エラーコールバックの追加
    void add_fail_callback(fail_fn fn) { _fail_callbacks.emplace_back(fn); }

    // アプレットリストを返す
    auto &get_applets() const { return _applets; }

    // アプレットを返す
    auto get_applet(const std::string &id) {
        for (auto &applet : _applets) {
            if (applet->name() == id) {
                return applet;
            }
        }
        return std::shared_ptr<applet>();
    }

    // アプレットを返す（テンプレート）
    template <class T>
    auto get_applet() {
        return std::static_pointer_cast<T>(get_applet(T::id));
    }

    // メインメニューを持つかどうか返す
    bool has_mainmenu() const { return _mainmenu_callbacks.size() > 0; }

protected:
    // 初期設定
    virtual void configure(sapp_desc &desc);

    // 初期設定 (imgui)
    virtual void configure_imgui(simgui_desc_t &desc) {}

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

protected:
    // 標準メニューの追加
    void add_standard_menus();

    // 標準アプレットの作成
    void make_standard_applets();

private:
    // 前回の時間
    uint64_t _last_time = 0;

    // 描画アクション
    sg_pass_action _pass_action;

    // アプレットリスト
    std::vector<std::shared_ptr<applet>> _applets;

    // コールバックリスト
    std::vector<init_fn> _init_callbacks;
    std::vector<mainmenu_fn> _mainmenu_callbacks;
    std::vector<background_fn> _background_callbacks;
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
