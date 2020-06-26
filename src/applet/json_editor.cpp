
#include "json_editor.h"

#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

namespace {

template <typename ... Args>
std::string format(const std::string& fmt, Args ... args ) {
    size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args ...);
    std::vector<char> buf(len + 1);
    std::snprintf(&buf[0], len + 1, fmt.c_str(), args ...);
    return std::string(&buf[0], &buf[0] + len);
}

void clean_json(nlohmann::json &json) {
    switch (json.type()) {
    case nlohmann::detail::value_t::null:
        json = nullptr;
        break;
    case nlohmann::detail::value_t::object:
        for (auto &elm : json) {
            clean_json(elm);
        }
        break;
    case nlohmann::detail::value_t::array:
        for (auto &elm : json) {
            clean_json(elm);
        }
        break;
    case nlohmann::detail::value_t::string:
        json = nlohmann::json::string_t {};
        break;
    case nlohmann::detail::value_t::boolean:
        json = nlohmann::json::boolean_t {};
        break;
    case nlohmann::detail::value_t::number_integer:
        json = nlohmann::json::number_integer_t {};
        break;
    case nlohmann::detail::value_t::number_unsigned:
        json = nlohmann::json::number_unsigned_t {};
        break;
    case nlohmann::detail::value_t::number_float:
        json = nlohmann::json::number_float_t {};
        break;
    case nlohmann::detail::value_t::binary:
        json = nlohmann::json::binary_t {};
        break;
    case nlohmann::detail::value_t::discarded:
        break;
    }
}

} // namespace

namespace applet {

void json_editor::install(dear::application *app) {
    app->add_frame_callback(std::bind(&json_editor::frame, this, std::placeholders::_1));

    _json = nlohmann::json::meta();
}

void json_editor::frame(double delta_time) {
    ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(name())) {
        if (_filename.empty()) {
            ImGui::Text("untitled.json");

        } else {
            ImGui::Text("%s", _filename.c_str());
        }
        ImGui::Separator();
        if (ImGui::BeginChild("json", ImVec2(-1, -1))) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
            ImGui::Columns(2);
            if (auto act = property("<ROOT>", _json); act && !_current_action) {
                _current_action = act;
            }
            ImGui::Columns(1);
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        if (_current_action) {
            apply_action();
        }
    }
    ImGui::End();
}

void json_editor::apply_action() {
    if (_current_action.target == nullptr) return;

    switch (_current_action.mode) {
    case action::mode_t::add:
    case action::mode_t::insert:
        ImGui::OpenPopup("New Object");
        break;
    case action::mode_t::clear:
        _current_action.target->clear();
        _current_action.reset();
        break;
    }

    if (ImGui::BeginPopupModal("New Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const auto is_array = _current_action.target->is_array();
        const auto is_object = _current_action.target->is_object();

        static int index = -1;
        static std::string name;
        if (is_array) {
            if (index < 0) index = _current_action.target->size();
            ImGui::SliderInt("index", &index, 0, _current_action.target->size());

        } else if (is_object) {
            ImGui::InputText("name", &name);
        }

        static const char *items[] = { "bool", "float", "int", "string", "object", "array", "null" };
        enum class object_type : int {
            boolean, floating, integer, string, object, array, null, unknown
        };
        static int object_type_index = static_cast<int>(object_type::unknown);
        if (static_cast<object_type>(object_type_index) == object_type::unknown) {
            if (is_array) {
                if (_current_action.target->size() == 0) {
                    object_type_index = 0;

                } else {
                    auto &elm = (*_current_action.target)[0];
                    if (elm.is_boolean()) {
                        object_type_index = static_cast<int>(object_type::boolean);
                        
                    } else if (elm.is_number_float()) {
                        object_type_index = static_cast<int>(object_type::floating);
                        
                    } else if (elm.is_number_integer()) {
                        object_type_index = static_cast<int>(object_type::integer);
                        
                    } else if (elm.is_string()) {
                        object_type_index = static_cast<int>(object_type::string);
                        
                    } else if (elm.is_object()) {
                        object_type_index = static_cast<int>(object_type::object);
                        
                    } else if (elm.is_array()) {
                        object_type_index = static_cast<int>(object_type::array);
                        
                    } else if (elm.is_null()) {
                        object_type_index = static_cast<int>(object_type::null);
                        
                    } else {
                        object_type_index = 0;
                    }
                }

            } else if (is_object) {
                object_type_index = 0;

            } else {
                object_type_index = 0;
            }
        }
        if (ImGui::ListBox("object type", &object_type_index, items, IM_ARRAYSIZE(items))) {

        }
        if (ImGui::Button("cancel")) {
            index = -1;
            name.clear();
            object_type_index = static_cast<int>(object_type::unknown);
            _current_action.reset();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (is_object && name.empty()) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
            button_color.w *= 0.5f;
            ImGui::PushStyleColor(ImGuiCol_Button, button_color);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }
        auto ok = ImGui::Button("ok");
        if (is_object && name.empty()) {
            ImGui::PopItemFlag();
            ImGui::PopStyleColor(2);
        }
        if (ok) {
            {
                auto &json = *_current_action.target;
                if (is_array) {
                    auto it = json.begin() + index;
                    switch (static_cast<object_type>(object_type_index)) {
                    case object_type::boolean:
                        json.insert(it, nlohmann::json::boolean_t {});
                        break;
                    case object_type::floating:
                        json.insert(it, nlohmann::json::number_float_t {});
                        break;
                    case object_type::integer:
                        json.insert(it, nlohmann::json::number_integer_t {});
                        break;
                    case object_type::string:
                        json.insert(it, nlohmann::json::string_t {});
                        break;
                    case object_type::object:
                        if (json.size() > 0 && json[0].is_object() && index != 0) {
                            auto inserted_it = json.insert(it, json[0]);
                            ::clean_json(*inserted_it);

                        } else {
                            json.insert(it, nlohmann::json::object());
                        }
                        break;
                    case object_type::array:
                        json.insert(it, nlohmann::json::array());
                        break;
                    case object_type::null:
                        json.insert(it, nullptr);
                        break;
                    }

                } else if (is_object) {
                    switch (static_cast<object_type>(object_type_index)) {
                    case object_type::boolean:
                        json[name] = nlohmann::json::boolean_t {};
                        break;
                    case object_type::floating:
                        json[name] = nlohmann::json::number_float_t {};
                        break;
                    case object_type::integer:
                        json[name] = nlohmann::json::number_integer_t {};
                        break;
                    case object_type::string:
                        json[name] = nlohmann::json::string_t {};
                        break;
                    case object_type::object:
                        json[name] = nlohmann::json::object();
                        break;
                    case object_type::array:
                        json[name] = nlohmann::json::array();
                        break;
                    case object_type::null:
                        json[name] = nullptr;
                        break;
                    }
                }
            }
            index = -1;
            name.clear();
            object_type_index = static_cast<int>(object_type::unknown);
            _current_action.reset();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

json_editor::action json_editor::property(const char *name, nlohmann::json &json) {
    action result_action;

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
        if (ImGui::InputTextWithHint("##value", "(empty)", &temp)) {
            json = temp;
        }
        end_leaf();

    } else if (json.is_object()) {
        auto open = begin_tree(name, "object", json.size(), result_action);
        if (result_action) {
            result_action.target = &json;
        }
        if (open) {
            for (auto it = json.begin(); it != json.end(); ++it) {
                if (auto child_action = property(it.key().c_str(), it.value()); child_action && !result_action) {
                    result_action = child_action;
                }
            }
            end_tree();
        }

    } else if (json.is_array()) {
        auto open = begin_tree(name, "array", json.size(), result_action);
        if (result_action) {
            result_action.target = &json;
        }
        if (open) {
            for (auto i = 0; i < json.size(); ++i) {
                if (auto child_action = property(::format("[%d]", i).c_str(), json[i]); child_action && !result_action) {
                    result_action = child_action;
                }
            }
            end_tree();
        }

    } else {
        begin_leaf(name);
        ImGui::Text("unknown");
        end_leaf();
    }

    if (ImGui::BeginPopupContextItem("item context menu"))
    {
        if (ImGui::Selectable("add")) result_action.mode = action::mode_t::add;
        if (ImGui::Selectable("remove")) result_action.mode = action::mode_t::remove;
        if (ImGui::Selectable("replace")) result_action.mode = action::mode_t::replace;
        if (ImGui::Selectable("move")) result_action.mode = action::mode_t::move;
        if (ImGui::Selectable("copy")) result_action.mode = action::mode_t::copy;
        if (ImGui::Selectable("clear")) result_action.mode = action::mode_t::clear;
        if (result_action) {
            result_action.target = &json;
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();

    return result_action;
}

void json_editor::begin_leaf(const char *name) {
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth);
    ImGui::OpenPopupOnItemClick("item context menu", ImGuiMouseButton_Right);
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(-1);
}

void json_editor::end_leaf() {
    ImGui::NextColumn();
}

bool json_editor::begin_tree(const char *name, const char *text, int size, action &act) {
    ImGui::AlignTextToFramePadding();
    bool node_open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth);
    ImGui::OpenPopupOnItemClick("item context menu", ImGuiMouseButton_Right);
    if (size > 0) {
        ImGui::SameLine();
        ImGui::TextDisabled("(%d)", size);
    }
    ImGui::NextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", text ? text : "unknown");
    ImGui::SameLine();
    
    ImGui::NextColumn();

    return node_open;
}

void json_editor::end_tree() {
    ImGui::TreePop();
}
} // namespace applet
