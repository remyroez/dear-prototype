#ifndef DEAR_CORE_APPLET_SETTING_H_
#define DEAR_CORE_APPLET_SETTING_H_

#include "applet.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace dear::core {

class applet_setting : public applet {
    DEAR_APPLET("Applet Setting##dear_core");
    
    // ウィンドウを開く
    virtual void open() override {
        applet::open();
        _selected = 0;
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
                ImGui::PushID(applet->name());
                if (!applet->has_window()) {
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
                }
                {
                    auto opened = applet->opened();
                    if (ImGui::Checkbox("##open", &opened)) {
                        applet->toggle();
                    }
                }
                if (!applet->has_window()) {
                    ImGui::PopStyleColor();
                    ImGui::PopItemFlag();
                }
                ImGui::SameLine();
                if (ImGui::Selectable(applet->name(), index == _selected)) {
                    _selected = index;
                }
                ImGui::PopID();
                index++;
            }
            ImGui::ListBoxFooter();
        }
        ImGui::NextColumn();

        // アプレット設定
        if (ImGui::BeginChild("right", ImVec2(-1, -1), false, ImGuiWindowFlags_NoScrollbar)) {
            dear::core::applet *applet = nullptr;
            if (_selected < 0) {
                // 選択したインデックスが不正

            } else if (_selected >= applets.size()) {
                // 選択したインデックスが超過

            } else {
                applet = applets[_selected].get();
            }
            if (applet) {
                ImGui::TextUnformatted(applet->name(), ImGui::FindRenderedTextEnd(applet->name()));

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
    int _selected = 0;
};

} // namespace dear::core

#endif // DEAR_CORE_APPLET_SETTING_H_
