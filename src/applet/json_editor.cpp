
#include "json_editor.h"

#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "stdio.h"

namespace {

// 文字列のフォーマット
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args ) {
    size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args ...);
    std::vector<char> buf(len + 1);
    std::snprintf(&buf[0], len + 1, fmt.c_str(), args ...);
    return std::string(&buf[0], &buf[0] + len);
}

// ＪＳＯＮの中身を構造を維持したまま空にする
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

// ＪＳＯＮパッチ add 操作のオブジェクトを作る
inline nlohmann::json make_add_op(const nlohmann::json::json_pointer &path, const nlohmann::json &value) {
    return { { "op", "add" }, { "path", path.to_string() }, { "value", value } };
}

// ＪＳＯＮパッチ remove 操作のオブジェクトを作る
inline nlohmann::json make_remove_op(const nlohmann::json::json_pointer &path) {
    return { { "op", "remove" }, { "path", path.to_string() } };
}

// ＪＳＯＮパッチ replace 操作のオブジェクトを作る
inline nlohmann::json make_replace_op(const nlohmann::json::json_pointer &path, const nlohmann::json &value) {
    return { { "op", "replace" }, { "path", path.to_string() }, { "value", value } };
}

// ＪＳＯＮパッチオブジェクトを作る
template<class... T>
inline nlohmann::json make_patch(T &&...ops) {
    return nlohmann::json::array({ std::forward<T>(ops)... });
}

// ＪＳＯＮパッチの削除操作を実行
void patch_remove(nlohmann::json &json, const nlohmann::json::json_pointer &pointer) {
    json = json.patch(make_patch(make_remove_op(pointer)));
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
            if (auto act = property("<ROOT>", _json, nlohmann::json::json_pointer()); act && !_current_action) {
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
    switch (_current_action.mode) {
    case action::mode_t::add:
    case action::mode_t::insert:
    case action::mode_t::replace:
        ImGui::OpenPopup("New Object");
        break;
    case action::mode_t::remove:
        ::patch_remove(_json, _current_action.pointer);
        _current_action.reset();
        break;
    case action::mode_t::clear:
        _json[_current_action.pointer].clear();
        _current_action.reset();
        break;
    defalut:
        _current_action.reset();
        break;
    }

    if (!_current_action) return;

    if (ImGui::BeginPopupModal("Object Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", _current_action.pointer.to_string().c_str());
        if (ImGui::Button("ok")) {
            _current_action.reset();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    window_new_object(_json, _current_action);
}

void json_editor::window_new_object(nlohmann::json &json, action &act) {
    if (!ImGui::BeginPopupModal("New Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

    // ターゲット
    auto &target = json[act.pointer];

    const auto is_add_op = act.mode == action::mode_t::add;
    const auto is_array = target.is_array();
    const auto is_object = target.is_object();

    // 添字（インデックス or キー）
    static int index = -1;
    static std::string name;
    if (is_array && is_add_op) {
        if (index < 0) index = target.size();
        ImGui::Text("%s", (act.pointer / index).to_string().c_str());
        ImGui::SliderInt("index", &index, 0, target.size());

    } else if (is_object && is_add_op) {
        ImGui::Text("%s", (act.pointer / name).to_string().c_str());
        ImGui::InputText("name", &name);

    } else {
        ImGui::Text("%s", act.pointer.to_string().c_str());
    }

    // オブジェクトタイプ
    static const char *items[] = { "bool", "float", "int", "string", "object", "array", "null" };
    enum class object_type : int {
        boolean, floating, integer, string, object, array, null, unknown
    };
    static int object_type_index = static_cast<int>(object_type::unknown);
    if (static_cast<object_type>(object_type_index) == object_type::unknown) {
        if (is_add_op && is_array) {
            if (target.size() == 0) {
                object_type_index = 0;

            } else {
                auto &elm = target[0];
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

        } else if (is_add_op && is_object) {
            object_type_index = 0;

        } else {
            object_type_index = 0;
            switch (target.type()) {
            case nlohmann::detail::value_t::null:
                object_type_index = static_cast<int>(object_type::null);
                break;
            case nlohmann::detail::value_t::object:
                object_type_index = static_cast<int>(object_type::object);
                break;
            case nlohmann::detail::value_t::array:
                object_type_index = static_cast<int>(object_type::array);
                break;
            case nlohmann::detail::value_t::string:
                object_type_index = static_cast<int>(object_type::string);
                break;
            case nlohmann::detail::value_t::boolean:
                object_type_index = static_cast<int>(object_type::boolean);
                break;
            case nlohmann::detail::value_t::number_integer:
            case nlohmann::detail::value_t::number_unsigned:
                object_type_index = static_cast<int>(object_type::integer);
                break;
            case nlohmann::detail::value_t::number_float:
                object_type_index = static_cast<int>(object_type::floating);
                break;
            case nlohmann::detail::value_t::binary:
                break;
            case nlohmann::detail::value_t::discarded:
                break;
            }
        }
    }
    if (ImGui::ListBox("object type", &object_type_index, items, IM_ARRAYSIZE(items))) {

    }

    // キャンセルボタン
    auto close = false;
    if (ImGui::Button("cancel")) {
        close = true;
    }

    ImGui::SameLine();

    // ＯＫボタン
    if (is_add_op && is_object && name.empty()) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        button_color.w *= 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Button, button_color);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }
    auto ok = ImGui::Button("ok");
    if (is_add_op && is_object && name.empty()) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor(2);
    }
    if (ok) {
        // ＪＳＯＮポインタ
        auto pointer = act.pointer;
        if (!is_add_op) {
            // replace 操作なら、そのものを指す

        } else if (is_array) {
            pointer /= index;

        } else if (is_object) {
            pointer /= name;
        }

        // 設定する値
        nlohmann::json value;
        switch (static_cast<object_type>(object_type_index)) {
        case object_type::boolean:
            value = nlohmann::json::boolean_t {};
            break;
        case object_type::floating:
            value = nlohmann::json::number_float_t {};
            break;
        case object_type::integer:
            value = nlohmann::json::number_integer_t {};
            break;
        case object_type::string:
            value = nlohmann::json::string_t {};
            break;
        case object_type::object:
            value = nlohmann::json::object();
            break;
        case object_type::array:
            value = nlohmann::json::array();
            break;
        case object_type::null:
            value = nullptr;
            break;
        }

        // 操作オブジェクト
        nlohmann::json op;
        switch (act.mode) {
        case action::mode_t::add:
            op = ::make_add_op(pointer, value);
            break;
        case action::mode_t::replace:
            op = ::make_replace_op(pointer, value);
            break;
        }

        // ＪＳＯＮパッチとして適用
        json = json.patch(::make_patch(op));
        
        close = true;
    }
    
    // ポップアップを閉じる
    if (close) {
        index = -1;
        name.clear();
        object_type_index = static_cast<int>(object_type::unknown);
        act.reset();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

json_editor::action json_editor::property(const char *name, nlohmann::json &json, nlohmann::json::json_pointer pointer) {
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
            result_action.pointer = pointer;
        }
        if (open) {
            for (auto it = json.begin(); it != json.end(); ++it) {
                if (auto child_action = property(it.key().c_str(), it.value(), pointer / it.key()); child_action && !result_action) {
                    result_action = child_action;
                }
            }
            end_tree();
        }

    } else if (json.is_array()) {
        auto open = begin_tree(name, "array", json.size(), result_action);
        if (result_action) {
            result_action.pointer = pointer;
        }
        if (open) {
            for (std::size_t i = 0; i < json.size(); ++i) {
                if (auto child_action = property(::format("%d", i).c_str(), json[i], pointer / i); child_action && !result_action) {
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
            result_action.pointer = pointer;
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
    if (size >= 0) {
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
