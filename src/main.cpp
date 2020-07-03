#include "dear.h"
#include "imgui.h"

#include "applet/background.h"
#include "applet/json_editor.h"

namespace {

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
        // 標準メインメニュー，アプレットを追加する
        dear::application::configure(desc);

        desc.enable_clipboard = true;
        desc.clipboard_size = 1024 * 1024;

        add_mainmenu_callback([this](auto){
            if (ImGui::BeginMenu("imgui")) {
                ImGui::MenuItem("Demo", nullptr, &_open_demo);
                ImGui::MenuItem("Metrics", nullptr, &_open_metrics);
                ImGui::MenuItem("Style Editor", nullptr, &_open_style);
                ImGui::Separator();
                ImGui::MenuItem("About Dear ImGui", nullptr, &_open_about);
                ImGui::EndMenu();
            }
        });
        add_frame_callback([this](auto){
            if (_open_demo) ImGui::ShowDemoWindow(&_open_demo);
            if (_open_about) ImGui::ShowAboutWindow(&_open_about);
            if (_open_metrics) ImGui::ShowMetricsWindow(&_open_metrics);
            if (_open_style) {
                ImGui::Begin("Dear ImGui Style Editor", &_open_style);
                ImGui::ShowStyleEditor();
                ImGui::End();
            }
        });
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
        make_applet<applet::background>();
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

        if (auto bg = get_applet<applet::background>()) {
            bg->load_background_image("avatar.png");
        }
    }

    dear::image _image;

    bool _open_demo = true;
    bool _open_about = false;
    bool _open_metrics = false;
    bool _open_style = false;
};

} // namespace

DEAR_MAIN(::application);
