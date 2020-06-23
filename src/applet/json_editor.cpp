
#include "json_editor.h"

#include "misc/cpp/imgui_stdlib.h"

namespace {

template <typename ... Args>
std::string format(const std::string& fmt, Args ... args )
{
    size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args ... );
    std::vector<char> buf(len + 1);
    std::snprintf(&buf[0], len + 1, fmt.c_str(), args ... );
    return std::string(&buf[0], &buf[0] + len);
}

} // namespace

namespace applet {

void json_editor::install(dear::application *app) {
    app->add_frame_callback(std::bind(&json_editor::frame, this, std::placeholders::_1));

    _json = nlohmann::json::object({
        {"pi", 3.141},
        {"happy", true},
        {"name", "Niels"},
        {"nothing", nullptr},
        {"answer", {
            {"everything", 42}
        }},
        {"list", {1, 0, 2}},
        {"object", {
            {"currency", "USD"},
            {"value", 42.99}
        }}
    });
}

void json_editor::frame(double delta_time) {
    ImGui::SetNextWindowSize(ImVec2(480, 640));
    if (ImGui::Begin(name())) {
        if (_filename.empty()) {
            ImGui::Text("unknown.json");

        } else {
            ImGui::Text("%s", _filename.c_str());
        }
        ImGui::Separator();
        if (ImGui::BeginChild("json", ImVec2(-1, -1))) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
            ImGui::Columns(2);
            property("<ROOT>", _json);
            ImGui::Columns(1);
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void json_editor::property(const char *name, nlohmann::json &json) {
    ImGui::PushID(name);
    ImGui::AlignTextToFramePadding();

    if (json.is_null()) {
        begin_leaf(name);
        ImGui::Text("null");
        end_leaf();

    } else if (json.is_boolean()) {
        begin_leaf(name);
        auto &data = json.get_ref<bool &>();
        ImGui::Checkbox("##value", &data);
        end_leaf();

    } else if (json.is_number_float()) {
        begin_leaf(name);
        auto temp = json.get<float>();
        if (ImGui::InputFloat("##value", &temp)) {
            json = temp;
        }
        end_leaf();

    } else if (json.is_number_integer()) {
        begin_leaf(name);
        auto temp = json.get<int>();
        if (ImGui::InputInt("##value", &temp)) {
            json = temp;
        }
        end_leaf();

    } else if (json.is_string()) {
        begin_leaf(name);
        auto temp = json.get<std::string>();
        if (ImGui::InputText("##value", &temp)) {
            json = temp;
        }
        end_leaf();

    } else if (json.is_object()) {
        if (begin_tree(name, "object")) {
            for (auto it = json.begin(); it != json.end(); ++it) {
                property(it.key().c_str(), it.value());
            }
#if 0
            {
                ImGui::PushID("add-button");
                ImGui::AlignTextToFramePadding();
                begin_leaf("");
                if (ImGui::SmallButton("add")) {

                }
                end_leaf();
                ImGui::PopID();
            }
#endif
            end_tree();
        }

    } else if (json.is_array()) {
        if (begin_tree(name, "array")) {
            for (auto i = 0; i < json.size(); ++i) {
                property(::format("[%d]", i).c_str(), json[i]);
            }
            end_tree();
        }

    } else {
        begin_leaf(name);
        ImGui::Text("unknown");
        end_leaf();
    }

    ImGui::PopID();
}

void json_editor::begin_leaf(const char *name) {
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(-1);
}

void json_editor::end_leaf() {
    ImGui::NextColumn();
}

bool json_editor::begin_tree(const char *name, const char *text) {
    ImGui::AlignTextToFramePadding();
    bool node_open = ImGui::TreeNode(name);
    ImGui::NextColumn();

    ImGui::AlignTextToFramePadding();
    ImGui::Text(text ? text : "unknown");
    ImGui::NextColumn();

    return node_open;
}

void json_editor::end_tree() {
    ImGui::TreePop();
}
} // namespace applet
