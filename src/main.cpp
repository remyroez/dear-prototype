#include "dear.h"
#include "imgui.h"

#include "applet/background.h"
#include "applet/json_editor.h"

namespace {

class applet_setting : public dear::applet {
    DEAR_APPLET("Applet Setting");
    
    // ウィンドウを開く
    virtual void open() override {
        applet::open();
        _selected = -1;
    }

    // ウィンドウ前処理
    virtual int pre_begin() override {
        ImGui::SetNextWindowSize(ImVec2(480, 320), ImGuiCond_FirstUseEver);
        return 0;
    }

    // ウィンドウコンテンツ処理
    virtual void content(double delta_time) override {
        ImGui::Columns(2);
        
        auto &applets = app()->get_applets();
        int index = 0;

        // アプレット一覧
        static auto first = true;
        if (first) {
            first = false;
            ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() / 3);
        }
        if (ImGui::ListBoxHeader("##list", ImVec2(-1, -1))) {
            for (auto &applet : applets) {
                if (ImGui::Selectable(applet->name(), index == _selected)) {
                    _selected = index;
                }
                index++;
            }
            ImGui::ListBoxFooter();
        }
        ImGui::NextColumn();

        // アプレット設定
        if (ImGui::BeginChild("right", ImVec2(-1, -1), false, ImGuiWindowFlags_NoScrollbar)) {
            dear::applet *applet = nullptr;
            if (_selected < 0) {
                // 選択したインデックスが不正

            } else if (_selected >= applets.size()) {
                // 選択したインデックスが超過

            } else {
                applet = applets[_selected].get();
            }
            if (applet) {
                ImGui::Text("%s", applet->name());

            } else {
                ImGui::TextDisabled("(no select)");
            }
            ImGui::Separator();
            if (ImGui::BeginChild("settings", ImVec2(-1, -1))) {
                if (applet) applet->settings();
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::Columns(1);
    }

    // 選択
    int _selected = -1;
};

class example_applet : public dear::applet {
    DEAR_APPLET("Example");

    // ウィンドウコンテンツ処理
    virtual void content(double delta_time) override {
        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        //ImGui::ColorEdit3("clear color", &_pass_action.colors[0].val[0]);
        if (ImGui::Button("Another Window")) show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window) {
            ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }
    }

    bool show_another_window = false;
};

class application : public dear::application {
    // 初期設定
    virtual void configure(sapp_desc &desc) override {
        desc.enable_clipboard = true;
        desc.clipboard_size = 1024 * 1024;

        add_mainmenu_callback([this](auto){
            if (ImGui::BeginMenu("dear")) {
                for (auto &applet : get_applets()) {
                    if (!applet->has_window()) {
                        // ウィンドウがないのでスキップ

                    } else if (ImGui::MenuItem(applet->name(), nullptr, applet->opened())) {
                        applet->toggle();
                    }
                }
                ImGui::EndMenu();
            }
        });
        add_frame_callback([](auto){ ImGui::ShowDemoWindow(); });
        add_frame_callback([this](auto){
            if (ImGui::Begin("image")) {
                ImGui::Text("width: %d", _image.width);
                ImGui::Text("height: %d", _image.height);
                ImGui::Text("channels: %d", _image.num_channels);
                ImGui::Image(
                    _image,
                    ImVec2(100, 100),
                    ImVec2(0.25f, 0.25f),
                    ImVec2(0.75f, 0.75f),
                    ImVec4(1.0f, 0.75f, 0.5f, 0.25f),
                    ImVec4(1.f, 1.f, 1.f, 1.f)
                );
            }
            ImGui::End();
        });
        make_applet<example_applet>(true);
        make_applet<applet_setting>();
        background = make_applet<applet::background>();
        make_applet<applet::json_editor>();
    }

    // 初期設定 (imgui)
    virtual void configure_imgui(simgui_desc_t &desc) override {
        desc.no_default_font = true;
    }

    // 初期化
    virtual void init() override {
        dear::gfx::load_font("NotoSansCJKjp-Regular.otf", 16);
        dear::gfx::build_font();

        dear::gfx::load_image_async("avatar.png", _image);

        if (auto app = background.lock(); auto bg = std::static_pointer_cast<applet::background>(app)) {
            bg->load_background_image("avatar.png");
        }
    }

    dear::image _image;

    applet_handle background;
};

} // namespace

DEAR_MAIN(::application);
