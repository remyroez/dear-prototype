
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
    app->add_mainmenu_callback([this](auto){
        if (ImGui::BeginMenu("background-size")) {
            if (ImGui::MenuItem("fixed", nullptr, _size == background_size::fixed)) _size = background_size::fixed;
            if (ImGui::MenuItem("fit", nullptr, _size == background_size::fit)) _size = background_size::fit;
            if (ImGui::MenuItem("cover", nullptr, _size == background_size::cover)) _size = background_size::cover;
            if (ImGui::MenuItem("contain", nullptr, _size == background_size::contain)) _size = background_size::contain;
            if (ImGui::BeginMenu("custom")) {
                if (ImGui::MenuItem("custom", nullptr, _size == background_size::custom)) _size = background_size::custom;
                ImGui::InputFloat2("uv0", &_custom_uv0.x);
                ImGui::InputFloat2("uv1", &_custom_uv1.x);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
    });
}

void background::load_background_image(const char *filename) {
    dear::gfx::load_image_async(filename, _image);
}

} // namespace applet
