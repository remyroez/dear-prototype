#include "core/application.h"

#include "imgui.h"

#include <unordered_map>

namespace {

class applets_applet : public dear::core::applet {
    // 名前
    virtual const char *name() override {
        return "applets";
    }

    // インストール
    virtual void install(dear::core::application *app) override {
        app->add_frame_callback(std::bind(&applets_applet::frame, this, app, std::placeholders::_1));
    }

    // フレーム経過
    void frame(dear::core::application *app, double delta_time) {
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

class example_applet : public dear::core::applet {
    // 名前
    virtual const char *name() override {
        return "example";
    }

    // インストール
    virtual void install(dear::core::application *app) override {
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

class application : public dear::core::application {
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
        make_applet<example_applet>();
        make_applet<applets_applet>();
    }
};

} // namespace

DEAR_MAIN(::application);
