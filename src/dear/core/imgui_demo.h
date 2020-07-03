#ifndef DEAR_CORE_IMGUI_DEMO_H_
#define DEAR_CORE_IMGUI_DEMO_H_

#include "applet.h"
#include "application.h"

#include "imgui.h"

namespace dear::core {

class imgui_demo : public applet {
    DEAR_APPLET("Dear ImGui Demo##dear_core");

    // コンストラクタ
    imgui_demo(bool open_demo, bool open_metrics, bool open_style, bool open_about)
    : _open_demo(open_demo), _open_metrics(open_metrics), _open_style(open_style), _open_about(open_about) {}

    // 初期化
    virtual void init() {
        app()->add_mainmenu_callback([this](auto){
            if (ImGui::BeginMenu("imgui")) {
                ImGui::MenuItem("Demo", nullptr, &_open_demo);
                ImGui::MenuItem("Metrics", nullptr, &_open_metrics);
                ImGui::MenuItem("Style Editor", nullptr, &_open_style);
                ImGui::Separator();
                ImGui::MenuItem("About Dear ImGui", nullptr, &_open_about);
                ImGui::EndMenu();
            }
        });
    }
    
    // フレーム経過
    virtual void frame(double delta_time) override {
        if (_open_demo) ImGui::ShowDemoWindow(&_open_demo);
        if (_open_about) ImGui::ShowAboutWindow(&_open_about);
        if (_open_metrics) ImGui::ShowMetricsWindow(&_open_metrics);
        if (_open_style) {
            ImGui::Begin("Dear ImGui Style Editor", &_open_style);
            ImGui::ShowStyleEditor();
            ImGui::End();
        }
    }

    // 各ウィンドウ表示フラグ
    bool _open_demo = false;
    bool _open_metrics = false;
    bool _open_style = false;
    bool _open_about = false;

public:
    // 各ウィンドウ表示フラグを設定する
    void open_demo(bool b = true) { _open_demo = b; }
    void open_metrics(bool b = true) { _open_metrics = b; }
    void open_style(bool b = true) { _open_style = b; }
    void open_about(bool b = true) { _open_about = b; }
};

} // namespace dear::core

#endif // DEAR_CORE_IMGUI_DEMO_H_
