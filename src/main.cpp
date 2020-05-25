#include "core/application.h"

#include "imgui.h"

namespace {

class example_applet : public dear::core::applet {
    // 名前
    virtual const char *name() override {
        return "example";
    }

    // 初期設定
    virtual void configure(dear::core::application *app) override {
        app->add_frame_callback(std::bind(&example_applet::frame, this, std::placeholders::_1));
    }

    // フレーム経過
    void frame(double delta_time) {
        static bool show_another_window = false;

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
        add_frame_callback([this](auto){
            ImGui::Begin("Applets");
            {
                if (ImGui::ListBoxHeader("##applets")) {
                    for (auto &applet : get_applets()) {
                        ImGui::Text("%s", applet->name());
                    }
                    ImGui::ListBoxFooter();
                }
            }
            ImGui::End();
        });
        add_frame_callback([](auto){ ImGui::ShowDemoWindow(); });
        make_applet<example_applet>();
    }
};

} // namespace

DEAR_MAIN(::application);
