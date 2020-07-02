
#include "background.h"

#include <filesystem>

#include "misc/cpp/imgui_stdlib.h"

namespace applet {

void background::install(dear::application *app) {
    app->add_background_callback([this](auto) {
        if (_enable_color) render_background_color();
        if (_enable_image) render_background_image();
    });
}

void background::render_background_color() {
    if (auto *drawlist = ImGui::GetBackgroundDrawList()) {
        if (_multi_color) {
            // マルチカラー
            drawlist->AddRectFilledMultiColor(
                drawlist->GetClipRectMin(),
                drawlist->GetClipRectMax(),
                _color_top_left,
                _color_top_right,
                _color_bottom_left,
                _color_bottom_right
            );

        } else {
            // シングルカラー
            drawlist->AddRectFilled(
                drawlist->GetClipRectMin(),
                drawlist->GetClipRectMax(),
                _color_top_left
            );
        }
    }
}

void background::render_background_image() {
    if (!_image) return;

    if (auto *drawlist = ImGui::GetBackgroundDrawList()) {
        // ＵＶ計算
        ImVec2 uv0, uv1;
        auto rect = drawlist->GetClipRectMax();
        calc_image_uv(rect, uv0, uv1);

        // 画像描画
        drawlist->AddImage(
            _image,
            drawlist->GetClipRectMin(),
            rect,
            uv0, uv1,
            _image_color
        );
    }
}

void background::calc_image_uv(const ImVec2 &rect, ImVec2 &uv0, ImVec2 &uv1) {
    switch (_size) {
    case background_size::fixed:
        dear::gfx::calc_uvs_fixed(
            _image.width, _image.height, rect.x, rect.y, uv0, uv1
        );
        break;

    case background_size::fit:
        uv0 = ImVec2(0.f, 0.f);
        uv1 = ImVec2(1.f, 1.f);
        break;

    case background_size::cover:
        dear::gfx::calc_uvs_cover(
            _image.width, _image.height, rect.x, rect.y, uv0, uv1
        );
        break;

    case background_size::contain:
        dear::gfx::calc_uvs_contain(
            _image.width, _image.height, rect.x, rect.y, uv0, uv1
        );
        break;

    case background_size::custom:
        uv0 = _custom_uv0;
        uv1 = _custom_uv1;
        break;
    }
}

void background::settings() {
    // 背景色
    if (ImGui::CollapsingHeader("background color", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 有効フラグ
        ImGui::Checkbox("enable##color", &_enable_color);

        ImGui::Checkbox("multi color", &_multi_color);
        if (_multi_color) {
            ImGui::ColorEdit3("top left", (float*)&_color_top_left);
            ImGui::ColorEdit3("top right", (float*)&_color_top_right);
            ImGui::ColorEdit3("bottom left", (float*)&_color_bottom_left);
            ImGui::ColorEdit3("bottom right", (float*)&_color_bottom_right);

        } else {
            ImGui::ColorEdit3("color###top left", (float*)&_color_top_left);
        }
        ImGui::Spacing();
    }
    
    // 背景画像
    if (ImGui::CollapsingHeader("background image", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 有効フラグ
        ImGui::Checkbox("enable##image", &_enable_image);

        // 背景画像サンプル
        ImVec2 uv0, uv1;
        {
            ImVec2 rect(ImGui::GetFrameHeight() * 4, ImGui::GetFrameHeight() * 3);
            calc_image_uv(rect, uv0, uv1);
            ImGui::Image(_image, rect, uv0, uv1, _image_color, ImColor(IM_COL32_WHITE));
        }
        ImGui::SameLine();
        {
            ImVec2 rect(ImGui::GetFrameHeight() * 3, ImGui::GetFrameHeight() * 4);
            calc_image_uv(rect, uv0, uv1);
            ImGui::Image(_image, rect, uv0, uv1, _image_color, ImColor(IM_COL32_WHITE));
        }

        // 画像の読み込み
        if (ImGui::InputTextWithHint("image", "(.png)", &_filepath, ImGuiInputTextFlags_EnterReturnsTrue)) {
            load_background_image(_filepath.c_str());
        }

        // 背景画像色
        ImGui::ColorEdit4("color", (float*)&_image_color);

        // 背景画像サイズ
        auto list_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetTextLineHeightWithSpacing() * 5 + ImGui::GetStyle().FramePadding.y);
        if (ImGui::ListBoxHeader("size", list_size)) {
            if (ImGui::Selectable("fixed", _size == background_size::fixed)) _size = background_size::fixed;
            if (ImGui::Selectable("fit", _size == background_size::fit)) _size = background_size::fit;
            if (ImGui::Selectable("cover", _size == background_size::cover)) _size = background_size::cover;
            if (ImGui::Selectable("contain", _size == background_size::contain)) _size = background_size::contain;
            if (ImGui::Selectable("custom", _size == background_size::custom)) _size = background_size::custom;
            ImGui::ListBoxFooter();
        }

        // カスタムＵＶ
        {
            auto edited = false;
            edited = ImGui::InputFloat2("custom uv0", &_custom_uv0.x) || edited;
            edited = ImGui::InputFloat2("custom uv1", &_custom_uv1.x) || edited;
            if (edited) {
                _size = background_size::custom;
            }
        }

        ImGui::Spacing();
    }
}

void background::load_background_image(const char *filename) {
    if (std::filesystem::exists(filename)) {
        dear::gfx::load_image_async(filename, _image);
        _filepath = filename;
    }
}

} // namespace applet
