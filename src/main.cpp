#include "dear.h"
#include "imgui.h"
#include "stb_image.h"

#include <unordered_map>

namespace {

class applets_applet : public dear::applet {
    // 名前
    virtual const char *name() override {
        return "applets";
    }

    // インストール
    virtual void install(dear::application *app) override {
        app->add_frame_callback(std::bind(&applets_applet::frame, this, app, std::placeholders::_1));
    }

    // フレーム経過
    void frame(dear::application *app, double delta_time) {
        ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Applets")) {
            int index = 0;
            for (auto &applet : app->get_applets()) {
                if (ImGui::Selectable(applet->name(), index == _selected)) {
                    _selected = index;
                }
                index++;
            }
        }
        ImGui::End();
    }

    // 選択
    int _selected = -1;
};

class example_applet : public dear::applet {
    // 名前
    virtual const char *name() override {
        return "example";
    }

    // インストール
    virtual void install(dear::application *app) override {
        app->add_frame_callback(std::bind(&example_applet::frame, this, std::placeholders::_1));
    }

    // フレーム経過
    void frame(double delta_time) {
        if (ImGui::Begin(name())) {
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
        ImGui::End();
    }

    bool show_another_window = false;
};

class application : public dear::application {
    // 背景サイズ
    enum background_size {
        fixed,
        fit,
        cover,
        contain,
    };
    background_size _size = background_size::contain;

    // 初期設定
    virtual void configure(sapp_desc &desc) override {
        desc.enable_clipboard = true;
        desc.clipboard_size = 1024 * 1024;

        set_background_color(0.5f, 0.3f, 0.1f);
        add_background_callback([this](auto) {
            const auto width = ImGui::GetWindowWidth();
            const auto height = ImGui::GetWindowHeight();
            const auto iw = static_cast<float>(_image.width > 0 ? _image.width : 1);
            const auto ih = static_cast<float>(_image.height > 0 ? _image.height : 1);
            const auto aspect_w = (width / height);
            const auto aspect_h = (height / width);
            const auto aspect_iw = (iw / ih);
            const auto aspect_ih = (ih / iw);
            const auto ratio_w = (iw / width);
            const auto ratio_h = (ih / height);
            ImVec2 uv0(0.f, 0.f);
            ImVec2 uv1(1.f, 1.f);
            switch (_size) {
            case background_size::fixed:
                uv1.x = width / iw;
                uv1.y = height / ih;
                break;
            case background_size::fit:
                uv1.x = 1.f;
                uv1.y = 1.f;
                break;
            case background_size::cover:
                uv1.x = width / iw;
                uv1.y = height / ih;
                break;
            case background_size::contain:
                if (aspect_w > aspect_iw) {
                    uv1.x = ratio_h / ratio_w;
                    uv1.y = 1.f;

                } else {
                    uv1.x = 1.f;
                    uv1.y = ratio_w / ratio_h;
                }
                break;
            }
            ImGui::Image(_image, ImVec2(width, height), uv0, uv1);
        });
        add_mainmenu_callback([this](auto){
            if (ImGui::BeginMenu("background-size")) {
                if (ImGui::MenuItem("fixed", nullptr, _size == background_size::fixed)) _size = background_size::fixed;
                if (ImGui::MenuItem("fit", nullptr, _size == background_size::fit)) _size = background_size::fit;
                if (ImGui::MenuItem("cover", nullptr, _size == background_size::cover)) _size = background_size::cover;
                if (ImGui::MenuItem("contain", nullptr, _size == background_size::contain)) _size = background_size::contain;
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
        make_applet<example_applet>();
        make_applet<applets_applet>();
    }

    // 初期設定 (imgui)
    virtual void configure_imgui(simgui_desc_t &desc) override {
        desc.no_default_font = true;
    }

    // 初期化
    virtual void init() override {
        dear::gfx::load_font("NotoSansCJKjp-Regular.otf", 16);
        dear::gfx::build_font();

        dear::gfx::load_image_async("bbb.png", _image);
    }

    dear::image _image;
};

} // namespace

DEAR_MAIN(::application);
