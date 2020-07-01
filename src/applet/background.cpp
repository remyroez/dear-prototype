
#include "background.h"

namespace applet {

void background::install(dear::application *app) {
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
