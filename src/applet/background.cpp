
#include "background.h"

namespace applet {

void background::install(dear::application *app) {
    app->add_background_callback([this](auto) {
        // 背景画像
        render_background_image();
    });
}

void background::render_background_image() {
    if (!_image) return;

    if (auto *drawlist = ImGui::GetBackgroundDrawList()) {
        // ＵＶ計算
        ImVec2 uv0(0.f, 0.f);
        ImVec2 uv1(1.f, 1.f);
        auto rect = drawlist->GetClipRectMax();
        switch (_size) {
        case background_size::fixed:
            dear::gfx::calc_uvs_fixed(
                _image.width, _image.height, rect.x, rect.y, uv0, uv1
            );
            break;
        case background_size::fit:
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

        // 画像描画
        drawlist->AddImage(
            _image,
            drawlist->GetClipRectMin(),
            rect,
            uv0, uv1
        );
    }
}

void background::settings() {
    if (ImGui::CollapsingHeader("background size", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::RadioButton("fixed", _size == background_size::fixed)) _size = background_size::fixed;
        if (ImGui::RadioButton("fit", _size == background_size::fit)) _size = background_size::fit;
        if (ImGui::RadioButton("cover", _size == background_size::cover)) _size = background_size::cover;
        if (ImGui::RadioButton("contain", _size == background_size::contain)) _size = background_size::contain;
        if (ImGui::RadioButton("custom", _size == background_size::custom)) _size = background_size::custom;
        ImGui::NewLine();
    }

    if (ImGui::CollapsingHeader("custom uv", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto edited = false;
        edited = ImGui::InputFloat2("uv0", &_custom_uv0.x) || edited;
        edited = ImGui::InputFloat2("uv1", &_custom_uv1.x) || edited;
        if (edited) {
            _size = background_size::custom;
        }
    }
}

void background::load_background_image(const char *filename) {
    dear::gfx::load_image_async(filename, _image);
}

} // namespace applet
