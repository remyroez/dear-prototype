#include "core/application.h"

#include "imgui.h"

namespace {

class example_applet : public dear::core::applet {
public:
    // フレーム経過
    virtual void frame(double delta_time) {
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
public:
    // 基底クラス
    using super = dear::core::application;

    // コンストラクタ継承
    using super::application;

protected:
    // 初期化
    virtual void init() override {
        set_background_color(0.5f, 0.3f, 0.1f);
        add_frame_callback([](auto){ ImGui::ShowDemoWindow(); });
        make_applet<example_applet>();
    }
};

} // namespace

DEAR_MAIN(::application);
