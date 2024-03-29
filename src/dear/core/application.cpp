#include "application.h"

#include "sokol_args.h"
#include "sokol_time.h"
#include "sokol_fetch.h"
#include "sokol_glue.h"

#include "imgui.h"
#include "imgui_internal.h"

#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

#include "applet_setting.h"
#include "imgui_demo.h"

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

// 背景の開始
bool begin_background() {
    const auto windowflags =
        ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoBackground;
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
    auto opened = ImGui::Begin("dear-background-window", nullptr, windowflags);
    if (opened) {
        ImGui::PopStyleVar(3);
        ImGui::DockSpace(ImGui::GetID("dear-background-dockspace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    }
    return opened;
}

// 背景の終了
void end_background() {
    ImGui::End();
}

} // namespace

namespace dear::core {

application::application() {
}

application::~application() {
    shutdown();
}

void application::setup(int argc, char** argv, sapp_desc &desc) {
    // 引数のセットアップ
    {
        sargs_desc args{ argc, argv };
        sargs_setup(&args);
    }

    // フェッチ機能のセットアップ
    {
        sfetch_desc_t desc{};
        //desc.max_requests = 1024;
        //desc.num_channels = 4;
        //desc.num_lanes = 8;
        sfetch_setup(&desc);
    }

    // 初期設定
    configure_cb(desc);
}

void application::shutdown() {
    sfetch_shutdown();
    sargs_shutdown();
}

void application::quit() {
    sapp_request_quit();
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

    // 初期カラー
    _pass_action.colors[0].action = SG_ACTION_CLEAR;
    _pass_action.colors[0].val[0] = 0.0f;
    _pass_action.colors[0].val[1] = 0.0f;
    _pass_action.colors[0].val[2] = 0.0f;
    _pass_action.colors[0].val[3] = 1.0f;

    // ユーザーコールバック
    configure(desc);

    // アプレット初期化
    for (auto &applet : _applets) {
        applet->reset_app(this);
        applet->install(this);
    }
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
    configure_imgui(simgui_desc);
    simgui_setup(&simgui_desc);
    {
        auto &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    // ユーザーコールバック
    init();

    // 登録済みコールバック
    for (auto &callback : _init_callbacks) {
        callback();
    }

    // アプレットコールバック
    for (auto &applet : _applets) {
        applet->init();
    }
}

void application::mainmenu_cb(double delta_time) {
    if (!has_mainmenu()) {
        // メインメニューを持たない

    } else if (ImGui::BeginMainMenuBar()) {
        // 登録済みコールバック
        for (auto &callback : _mainmenu_callbacks) {
            callback(delta_time);
        }

        ImGui::EndMainMenuBar();
    }
}

void application::frame_cb() {
    // フェッチ更新
    sfetch_dowork();

    // imgui 新フレーム
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = stm_sec(stm_laptime(&_last_time));
    simgui_new_frame(width, height, delta_time);

    // メインメニュー
    mainmenu_cb(delta_time);

    // 背景
    ::begin_background();
    for (auto &callback : _background_callbacks) {
        callback(delta_time);
    }
    ::end_background();

    // ユーザーコールバック
    frame(delta_time);

    // 登録済みコールバック
    for (auto &callback : _frame_callbacks) {
        callback(delta_time);
    }

    // アプレットコールバック
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
    // アプレット破棄
    for (auto &applet : _applets) {
        applet->uninstall(this);
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

void application::configure(sapp_desc &desc) {
    add_standard_menus();
    make_standard_applets();
}

void application::add_standard_menus() {
    add_mainmenu_callback([this](auto){
        if (ImGui::BeginMenu("dear##dear_core_application")) {
            for (auto &applet : get_applets()) {
                if (!applet->has_window()) {
                    // ウィンドウがないのでスキップ

                } else if (ImGui::MenuItem(applet->name(), nullptr, applet->opened())) {
                    applet->toggle();
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                quit();
            }
            ImGui::EndMenu();
        }
    });
}

void application::make_standard_applets() {
    make_applet<applet_setting>();
    make_applet<imgui_demo>();
}

} // namespace dear::core
