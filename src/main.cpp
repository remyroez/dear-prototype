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
        ImGui::Begin("Applets");
        {
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
        set_background_color(0.5f, 0.3f, 0.1f);
        add_mainmenu_callback([this](auto){
            if (ImGui::BeginMenu("foo")) {
                if (ImGui::MenuItem("bar")) {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("baz")) {
                }
                ImGui::EndMenu();
            }
        });
        add_frame_callback([](auto){ ImGui::ShowDemoWindow(); });
        add_frame_callback([this](auto){
            if (ImGui::Begin("image")) {
                ImGui::Image(reinterpret_cast<ImTextureID>(_image.id), ImVec2(100, 100));
                ImGui::End();
            }
        });
        make_applet<example_applet>();
        make_applet<applets_applet>();
    }

    // 初期化
    virtual void init() override {
        int x, y, c;
        if (auto *data = stbi_load("avatar.png", &x, &y, &c, 4)) {
            sg_image_desc desc{};
            desc.width = x;
            desc.height = y;
            desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            desc.min_filter = SG_FILTER_LINEAR;
            desc.mag_filter = SG_FILTER_LINEAR;
            desc.content.subimage[0][0].ptr = data;
            desc.content.subimage[0][0].size = x * y * c;
            _image = sg_alloc_image();
            sg_init_image(_image, &desc);
            stbi_image_free(data);

        } else {
            uint32_t pixels[] = {
                0xFFFF00FF,
            };
            sg_image_desc desc{};
            desc.width = 1;
            desc.height = 1;
            desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            desc.content.subimage[0][0].ptr = pixels;
            desc.content.subimage[0][0].size = sizeof(pixels);
            _image = sg_make_image(&desc);
        }
    }

    sg_image _image;
};

} // namespace

DEAR_MAIN(::application);
