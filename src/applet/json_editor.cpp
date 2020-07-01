
#include "json_editor.h"

#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include <filesystem>
#include <fstream>

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
    case nlohmann::detail::value_t::object:
    case nlohmann::detail::value_t::array:
        for (auto &elm : json) {
            clean_json(elm);
        }
        break;
    default:
        json.clear();
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
    ImGui::PushID(this);
    ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(name(), nullptr, ImGuiWindowFlags_MenuBar)) {
        menubar();

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

        if (auto maybe_path = popup_file_dialog("Open File##json_editor")) {
            open_file(*maybe_path);
        }

        if (auto maybe_path = popup_file_dialog("Save File##json_editor")) {
            save_file(*maybe_path);
        }
    }
    ImGui::End();
    ImGui::PopID();
}

void json_editor::menubar() {
    if (!ImGui::BeginMenuBar()) return;

    enum class menu_command {
        none,
        open,
        save_as,
    };
    menu_command command = menu_command::none;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New")) {
            new_file();
        }
        if (ImGui::MenuItem("Open", "Ctrl+O")) {
            command = menu_command::open;
        }
        if (ImGui::MenuItem("Save", nullptr, nullptr, !_filepath.empty())) {
            save_file(_filepath);
        }
        if (ImGui::MenuItem("Save As...")) {
            command = menu_command::save_as;
        }
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();

    switch (command) {
    case menu_command::open:
        ImGui::OpenPopup("Open File##json_editor");
        break;
    case menu_command::save_as:
        ImGui::OpenPopup("Save File##json_editor");
        break;
    }
}

void json_editor::new_file() {
    _filepath.clear();
    _filename.clear();
    _json.clear();
    _current_action.reset();
}

void json_editor::open_file(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) {
        // ファイルが見つからない

    } else if (!std::filesystem::is_regular_file(path)) {
        // 通常のファイルではない

    } else if (std::ifstream ifs(path.string()); !ifs.is_open()) {
        // ファイルを開くことに失敗した
        
    } else {
        // ファイル読み込み
        ifs >> _json;
        _filepath = path;
        _filename = path.filename();
    }
}

void json_editor::save_file(const std::filesystem::path &path) {
    std::ofstream ofs(path.string());
    ofs << std::setw(4) << _json << std::endl;

    _filename = path.filename();
    _filepath = path;
}

std::optional<std::filesystem::path> json_editor::popup_file_dialog(const char *id) {
    ImGui::SetNextWindowSize(ImVec2(480, 320), ImGuiCond_FirstUseEver);
    if (!ImGui::BeginPopupModal(id, nullptr)) return std::nullopt;

    auto ok = false;
    static auto update = true;

    // ディレクトリ
    static std::filesystem::path directory = std::filesystem::current_path();
    if (ImGui::ArrowButton("back", ImGuiDir_Left)) {
        if (auto parent_path = directory.parent_path(); parent_path != directory) {
            directory = parent_path;
            update = true;
        }
    }
    ImGui::SameLine();
    {
        auto temp = directory.string();
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##directory", &temp, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();
    }

    // ファイル名
    static std::string filename;
    static std::string filter = ".json";

    // ファイル一覧
    static std::vector<std::filesystem::directory_entry> files;
    static int index = -1;
    if (update) {
        files.clear();
        for (auto &entry : std::filesystem::directory_iterator{ directory }) {
            if (!entry.is_directory() && !filter.empty() && entry.path().extension() != filter) {
                // フィルタ一致しない

            } else {
                files.push_back(entry);
            }
        }
        index = -1;
        update = false;
    }
    if (ImGui::ListBoxHeader("##files", ImVec2(-1, -(ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing() * 3)))) {
        int i = 0;
        for (auto &file : files) {
            if (file.is_directory()) {
                if (ImGui::Selectable((file.path().filename().string() + "/").c_str(), index == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                    index = i;
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        directory = file.path();
                        update = true;
                    }
                }
            }
            ++i;
        }
        i = 0;
        for (auto &file : files) {
            if (file.is_directory()) {
                // 

            } else {
                if (ImGui::Selectable(file.path().filename().c_str(), index == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                    index = i;
                    filename = file.path().filename();
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        ok = true;
                    }
                }
            }
            ++i;
        }
        ImGui::ListBoxFooter();
    }

    ImGui::InputText("file name", &filename);
    if (ImGui::InputText("filter", &filter)) {
        update = true;
    }

    std::filesystem::path result;
    bool close = false;
    if (ImGui::Button("cancel")) {
        close = true;
    }
    ImGui::SameLine();

    if (filename.empty()) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        button_color.w *= 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Button, button_color);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }
    ok = ImGui::Button("ok") || ok;
    if (filename.empty()) {
        ImGui::PopItemFlag();
        ImGui::PopStyleColor(2);
    }
    if (ok) {
        result = directory / filename;
        close = true;
    }

    if (close) {
        update = true;
        filename.clear();
        filter = ".json";
        directory = std::filesystem::current_path();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();

    return !result.empty() ? std::optional<std::filesystem::path>{ result } : std::nullopt;
}

void json_editor::apply_action() {
    switch (_current_action.mode) {
    case action::mode_t::add:
    case action::mode_t::insert:
    case action::mode_t::replace:
        ImGui::OpenPopup("New Object##json_editor");
        break;
    case action::mode_t::remove:
        if (_current_action.pointer.empty()) {
            // ルートは削除できない

        } else {
            ::patch_remove(_json, _current_action.pointer);
        }
        _current_action.reset();
        break;
    case action::mode_t::copy:
        sapp_set_clipboard_string(_json[_current_action.pointer].dump().c_str());
        _current_action.reset();
        break;
    case action::mode_t::paste:
        if (auto parsed = nlohmann::json::parse(sapp_get_clipboard_string(), nullptr, false); !parsed.is_discarded()) {
            _json[_current_action.pointer] = parsed;
        }
        _current_action.reset();
        break;
    case action::mode_t::clear:
        _json[_current_action.pointer].clear();
        _current_action.reset();
        break;
    default:
        _current_action.reset();
        break;
    }

    if (!_current_action) return;

    if (ImGui::BeginPopupModal("Object Info##json_editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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
    if (!ImGui::BeginPopupModal("New Object##json_editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

    static auto first = true;

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

    // 設定する値
    static nlohmann::json new_value;

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
    if (ImGui::ListBox("object type", &object_type_index, items, IM_ARRAYSIZE(items)) || first) {
        switch (static_cast<object_type>(object_type_index)) {
        case object_type::boolean:
            if (target.is_boolean()) {
                new_value = target;

            } else {
                new_value = nlohmann::json::boolean_t {};
            }
            break;
        case object_type::floating:
            if (target.is_number_float()) {
                new_value = target;

            } else {
                new_value = nlohmann::json::number_float_t {};
            }
            break;
        case object_type::integer:
            if (target.is_number_integer()) {
                new_value = target;

            } else {
                new_value = nlohmann::json::number_integer_t {};
            }
            break;
        case object_type::string:
            if (target.is_string()) {
                new_value = target;

            } else {
                new_value = nlohmann::json::string_t {};
            }
            break;
        case object_type::object:
            if (target.is_array() && target.size() > 0 && target[0].is_object()) {
                new_value = target[0];
                clean_json(new_value);

            } else {
                new_value = nlohmann::json::object();
            }
            break;
        case object_type::array:
            if (target.is_array() && target.size() > 0 && target[0].is_array()) {
                new_value = target[0];
                clean_json(new_value);

            } else {
                new_value = nlohmann::json::array();
            }
            break;
        case object_type::null:
            new_value = nullptr;
            break;
        }
    }

    value("value", new_value);

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

        // 操作オブジェクト
        nlohmann::json op;
        switch (act.mode) {
        case action::mode_t::add:
            op = ::make_add_op(pointer, new_value);
            break;
        case action::mode_t::replace:
            op = ::make_replace_op(pointer, new_value);
            break;
        }

        // ＪＳＯＮパッチとして適用
        json = json.patch(::make_patch(op));
        
        close = true;
    }

    first = false;

    // ポップアップを閉じる
    if (close) {
        first = true;
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

    if (json.is_object()) {
        if (begin_tree(name, json)) {
            for (auto it = json.begin(); it != json.end(); ++it) {
                if (auto child_action = property(it.key().c_str(), it.value(), pointer / it.key()); child_action && !result_action) {
                    result_action = child_action;
                }
            }
            end_tree();
        }

    } else if (json.is_array()) {
        if (begin_tree(name, json)) {
            for (std::size_t i = 0; i < json.size(); ++i) {
                if (auto child_action = property(::format("%d", i).c_str(), json[i], pointer / i); child_action && !result_action) {
                    result_action = child_action;
                }
            }
            end_tree();
        }

    } else {
        begin_leaf(name);
        value(json);
        end_leaf();
    }

    // コンテキストメニュー
    if (ImGui::BeginPopup("item context menu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
    {
        if (ImGui::Selectable("add")) result_action.mode = action::mode_t::add;
        if (ImGui::Selectable("replace")) result_action.mode = action::mode_t::replace;
        //if (ImGui::Selectable("move")) result_action.mode = action::mode_t::move;
        ImGui::Separator();
        if (ImGui::Selectable("copy")) result_action.mode = action::mode_t::copy;
        if (ImGui::Selectable("paste")) result_action.mode = action::mode_t::paste;
        ImGui::Separator();
        if (ImGui::Selectable("clear")) result_action.mode = action::mode_t::clear;
        if (!pointer.empty() && ImGui::Selectable("remove")) result_action.mode = action::mode_t::remove;
        if (result_action) {
            result_action.pointer = pointer;
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();

    return result_action;
}

void json_editor::value(const char *id, nlohmann::json &json) {
    if (json.is_null()) {
        ImGui::Text("null");

    } else if (json.is_boolean()) {
        auto &data = json.get_ref<bool &>();
        ImGui::Checkbox(id, &data);

    } else if (json.is_number_float()) {
        auto temp = json.get<float>();
        if (ImGui::InputFloat(id, &temp)) {
            json = temp;
        }

    } else if (json.is_number_integer()) {
        auto temp = json.get<int>();
        if (ImGui::InputInt(id, &temp)) {
            json = temp;
        }

    } else if (json.is_string()) {
        auto temp = json.get<std::string>();
        if (ImGui::InputTextWithHint(id, "(empty)", &temp)) {
            json = temp;
        }

    } else if (json.is_object()) {
        ImGui::TextDisabled("object");

    } else if (json.is_array()) {
        ImGui::TextDisabled("array");

    } else {
        ImGui::Text("unknown");
    }
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

bool json_editor::begin_tree(const char *name, nlohmann::json &json) {
    ImGui::AlignTextToFramePadding();
    bool node_open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth);
    ImGui::OpenPopupOnItemClick("item context menu", ImGuiMouseButton_Right);
    if (json.is_array() || json.is_object()) {
        ImGui::SameLine();
        ImGui::TextDisabled("(%lu)", json.size());
    }
    ImGui::NextColumn();

    ImGui::AlignTextToFramePadding();
    value(json);
    ImGui::NextColumn();

    return node_open;
}

void json_editor::end_tree() {
    ImGui::TreePop();
}
} // namespace applet
