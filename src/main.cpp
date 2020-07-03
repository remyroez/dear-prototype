#include "dear.h"
#include "imgui.h"

#include "applet/background.h"
#include "applet/json_editor.h"

namespace {

class example_applet : public dear::applet {
    DEAR_APPLET("Example");

    // ウィンドウコンテンツ処理
    virtual void content(double delta_time) override {
        // Our state
        static bool show_another_window = false;
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        // NOTE: Show in imgui_demo applet
        //if (show_demo_window)
        //    ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            //ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            if (auto demo = app()->get_applet<dear::core::imgui_demo>())
            {
                bool show_demo_window = demo->open_demo();
                if (ImGui::Checkbox("Demo Window", &show_demo_window)) // Edit bools storing our window open/close state
                {
                    demo->toggle_demo();
                }
            }
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
    }
};

class application : public dear::application {
    // 初期設定
    virtual void configure(sapp_desc &desc) override {
        // 標準メインメニュー，アプレットを追加する
        dear::application::configure(desc);

        desc.enable_clipboard = true;
        desc.clipboard_size = 1024 * 1024;

        add_frame_callback([this](auto){
            ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_FirstUseEver);
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
        if (auto apl = get_applet<dear::core::imgui_demo>()) {
            apl->open_demo(true);
        }
    }

    dear::image _image;
};

} // namespace

DEAR_MAIN(::application);
