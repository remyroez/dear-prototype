#include "dear.h"
#include "imgui.h"
#include "stb_image.h"

#include <unordered_map>

namespace {

class background_applet : public dear::applet {
    // 名前
    virtual const char *name() override {
        return "background";
    }

    // インストール
    virtual void install(dear::application *app) override {
        app->add_background_callback([this](auto) {
            if (!_image) return;

            switch (_size) {
            case background_size::fixed:
                dear::gfx::render_image_fixed(_image, ImGui::GetWindowSize());
                break;
            case background_size::fit:
                ImGui::Image(_image, ImGui::GetWindowSize());
                break;
            case background_size::cover:
                dear::gfx::render_image_cover(_image, ImGui::GetWindowSize());
                break;
            case background_size::contain:
                dear::gfx::render_image_contain(_image, ImGui::GetWindowSize());
                break;
            case background_size::custom:
                ImGui::Image(_image, ImGui::GetWindowSize(), _custom_uv0, _custom_uv1);
                break;
            }
        });
        app->add_mainmenu_callback([this](auto){
            if (ImGui::BeginMenu("background-size")) {
                if (ImGui::MenuItem("fixed", nullptr, _size == background_size::fixed)) _size = background_size::fixed;
                if (ImGui::MenuItem("fit", nullptr, _size == background_size::fit)) _size = background_size::fit;
                if (ImGui::MenuItem("cover", nullptr, _size == background_size::cover)) _size = background_size::cover;
                if (ImGui::MenuItem("contain", nullptr, _size == background_size::contain)) _size = background_size::contain;
                if (ImGui::BeginMenu("custom")) {
                    if (ImGui::MenuItem("custom", nullptr, _size == background_size::custom)) _size = background_size::custom;
                    ImGui::InputFloat2("uv0", &_custom_uv0.x);
                    ImGui::InputFloat2("uv1", &_custom_uv1.x);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
        });
    }

    // 背景画像
    dear::image _image;

    // 背景サイズ列挙型
    enum background_size {
        fixed,
        fit,
        cover,
        contain,
        custom,
    };

    // 背景サイズ
    background_size _size = background_size::fixed;

    // UV
    ImVec2 _custom_uv0 { 0.f, 0.f };
    ImVec2 _custom_uv1 { 1.f, 1.f };

public:
    // 背景画像の読み込み
    void load_background_image(const char *filename) {
        dear::gfx::load_image_async(filename, _image);
    }

    // 背景画像のラップの設定
    void set_background_image_wrap(sg_wrap wrap_u = _SG_WRAP_DEFAULT, sg_wrap wrap_v = _SG_WRAP_DEFAULT) {
        _image.wrap_u = wrap_u;
        _image.wrap_v = wrap_v;
    }

    // 背景サイズの設定
    void set_background_size(background_size size) {
        _size = size;
    }

    // 背景サイズの設定 (UV指定)
    void set_background_size(const ImVec2 &uv0 = ImVec2(0.f, 0.f), const ImVec2 &uv1 = ImVec2(1.f, 1.f)) {
        set_background_size(background_size::custom);
        _custom_uv0 = uv0;
        _custom_uv1 = uv1;
    }
};

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
        custom,
    };
    background_size _size = background_size::fixed;

    // 初期設定
    virtual void configure(sapp_desc &desc) override {
        desc.enable_clipboard = true;
        desc.clipboard_size = 1024 * 1024;

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
        background = make_applet<background_applet>();
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

        if (auto app = background.lock(); auto bg = std::static_pointer_cast<background_applet>(app)) {
            bg->load_background_image("avatar.png");
        }
    }

    dear::image _image;

    applet_handle background;
};

} // namespace

DEAR_MAIN(::application);
