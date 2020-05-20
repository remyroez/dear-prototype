#include "application.h"

#include "sokol_args.h"
#include "sokol_time.h"
#include "sokol_glue.h"

#include "imgui.h"

#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

namespace {

// init コールバック
void init(void *userdata) {
    if (auto *app = static_cast<dear::core::application *>(userdata)) {
        app->init_cb();
    }
}

// frame コールバック
void frame(void *userdata) {
    if (auto *app = static_cast<dear::core::application *>(userdata)) {
        app->frame_cb();
    }
}

// cleanup コールバック
void cleanup(void *userdata) {
    if (auto *app = static_cast<dear::core::application *>(userdata)) {
        app->cleanup_cb();
    }
}

// event コールバック
void event(const sapp_event* ev, void *userdata) {
    if (auto *app = static_cast<dear::core::application *>(userdata)) {
        app->event_cb(ev);
    }
}

// fail コールバック
void fail(const char *message, void *userdata) {
    if (auto *app = static_cast<dear::core::application *>(userdata)) {
        app->fail_cb(message);
    }
}

} // namespace

namespace dear::core {

application::application(int argc, char** argv) {
    // 引数のセットアップ
    sargs_desc desc{ argc, argv };
    sargs_setup(&desc);
}

application::~application() {
    sargs_shutdown();
}

void application::configure_cb(sapp_desc &desc) {
    // デフォルト設定
    desc.user_data = this;
    desc.init_userdata_cb = ::init;
    desc.frame_userdata_cb = ::frame;
    desc.cleanup_userdata_cb = ::cleanup;
    desc.event_userdata_cb = ::event;
    desc.fail_userdata_cb = ::fail;
    desc.width = 1024;
    desc.height = 768;
    desc.gl_force_gles2 = true;
    desc.window_title = "dear";
    desc.ios_keyboard_resizes_canvas = false;

    // ユーザーコールバック
    configure(desc);
}

void application::init_cb() {
    // gfx
    sg_desc desc = {};
    desc.context = sapp_sgcontext();
    sg_setup(&desc);

    // time
    stm_setup();

    // imgui
    simgui_desc_t simgui_desc = {};
    simgui_setup(&simgui_desc);

    // 初期カラー
    _pass_action.colors[0].action = SG_ACTION_CLEAR;
    _pass_action.colors[0].val[0] = 0.0f;
    _pass_action.colors[0].val[1] = 0.5f;
    _pass_action.colors[0].val[2] = 0.7f;
    _pass_action.colors[0].val[3] = 1.0f;

    // ユーザーコールバック
    init();

    // 登録済みコールバック
    for (auto &callback : _init_callbacks) {
        callback();
    }

    // アプレット初期化
    for (auto &applet : _applets) {
        applet->init();
    }
}

void application::frame_cb() {
    // imgui 新フレーム
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = stm_sec(stm_laptime(&_last_time));
    simgui_new_frame(width, height, delta_time);

    // ユーザーコールバック
    frame(delta_time);

    // 登録済みコールバック
    for (auto &callback : _frame_callbacks) {
        callback(delta_time);
    }

    // アプレット描画
    for (auto &applet : _applets) {
        applet->frame(delta_time);
    }

    // 画面クリア
    sg_begin_default_pass(&_pass_action, width, height);

    // imgui レンダリング
    simgui_render();

    // 描画
    sg_end_pass();
    sg_commit();
}

void application::cleanup_cb() {
    // アプレットクリーンアップ
    for (auto &applet : _applets) {
        applet->cleanup();
    }

    // 登録済みコールバック
    for (auto &callback : _cleanup_callbacks) {
        callback();
    }

    // ユーザーコールバック
    cleanup();

    // sokol シャットダウン
    simgui_shutdown();
    sg_shutdown();
}

void application::event_cb(const sapp_event *ev) {
    // imgui ハンドリング
    if (simgui_handle_event(ev)) {
        // imgui 側が入力された

    } else if (event(ev)) {
        // ユーザーコールバック
        
    } else {
        // 登録済みコールバック
        for (auto &callback : _event_callbacks) {
            if (callback(ev)) return;
        }

        // アプレットイベント
        for (auto &applet : _applets) {
            if (applet->event(ev)) return;
        }
    }
}

void application::fail_cb(const char *message) {
    //SOKOL_LOG(message);
    
    // 登録済みコールバック
    for (auto &callback : _fail_callbacks) {
        callback(message);
    }

    // ユーザーコールバック
    fail(message);
}

} // namespace dear::core
