#ifndef DEAR_CORE_APPLET_H_
#define DEAR_CORE_APPLET_H_

#include "sokol_app.h"

namespace dear::core {

class application;

class applet {
public:
    // コンストラクタ
    applet(bool open = false): _opened(open) {}

    // デストラクタ
    virtual ~applet() {}

    // 名前
    virtual const char *name() = 0;

    // ウィンドウがあるかどうか返す
    virtual bool has_window() const { return true; }

    // インストール
    virtual void install(application *app) {}

    // アンインストール
    virtual void uninstall(application *app) {}

    // 設定
    virtual void settings() {}

    // ウィンドウを開く
    virtual void open() { _opened = true; }

    // ウィンドウを閉じる
    virtual void close() { _opened = false; }

    // ウィンドウを開閉
    virtual void toggle() {
        if (!has_window()) {
            // ウィンドウがないので開閉しない

        } else if (opened()) {
            close();

        } else {
            open();
        }
    }

    // ウィンドウを開いているかどうか返す
    virtual bool opened() { return _opened; }

    // 初期化
    virtual void init() {}

    // フレーム経過
    virtual void frame(double delta_time) {
        if (!has_window()) return;

        auto flags = pre_begin();
        if (begin(flags)) {
            content(delta_time);
        }
        end();
    }

    // アプリケーションを設定する
    void reset_app(application *new_app) { _app = new_app; }

protected:
    // ウィンドウ前処理
    virtual int pre_begin() { return 0; }

    // ウィンドウ開始処理
    virtual bool begin(int flags = 0);

    // ウィンドウコンテンツ処理
    virtual void content(double delta_time) {}

    // ウィンドウ終了処理
    virtual void end();

    // アプリケーションを返す
    application *app() const { return _app; }

protected:
    // アプリケーション
    application *_app = nullptr;

    // ウィンドウ開閉フラグ
    bool _opened = false;
};

} // namespace dear::core

// アプレット定義マクロ
#define DEAR_APPLET(NAME) \
public:\
static constexpr const char *id = NAME; \
private:\
using applet::applet; \
virtual const char *name() override { return NAME; }

//あアプレット定義マクロ（ウィンドウ無し）
#define DEAR_APPLET_NOWIN(NAME) \
DEAR_APPLET(NAME) \
virtual bool has_window() const override { return false; }

#endif // DEAR_CORE_APPLET_H_
