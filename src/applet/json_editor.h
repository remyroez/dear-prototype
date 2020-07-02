#ifndef APPLET_JSON_H_
#define APPLET_JSON_H_

#include "dear.h"
#include "json.hpp"

#include <string>
#include <filesystem>
#include <optional>

namespace applet {

class json_editor : public dear::applet {
    DEAR_APPLET("JSON Editor");
    
    // ウィンドウを開く
    virtual void open() override {
        applet::open();
        new_file();
    }

    // ウィンドウ前処理
    virtual int pre_begin() override;

    // ウィンドウコンテンツ処理
    virtual void content(double delta_time) override;

    // メニューバー
    void menubar();

    // 新規ファイル
    void new_file();

    // ファイルを開く
    void open_file(const std::filesystem::path &path);

    // ファイルを保存
    void save_file(const std::filesystem::path &path);

    // ファイルダイアログのポップアップ
    static std::optional<std::filesystem::path> popup_file_dialog(const char *id);

    // アクション適用
    void apply_action();

    // アクション
    struct action {
        // モード列挙型
        enum class mode_t {
            none,
            add,
            remove,
            replace,
            move,
            copy,
            paste,
            insert,
            clear,
        };

        // モード
        mode_t mode = mode_t::none;

        // ＪＳＯＮポインタ
        nlohmann::json::json_pointer pointer;

        // リセット
        void reset() {
            mode = mode_t::none;
            pointer = nlohmann::json::json_pointer();
        }

        // オペレーター: 代入
        action &operator =(const action &other) {
            mode = other.mode;
            pointer = other.pointer;
            return *this;
        }

        // オペレーター: キャスト(bool) … アクションがあるかどうか
        operator bool() const { return mode != mode_t::none; }
    };

    // 新規オブジェクトウィンドウ
    static void window_new_object(nlohmann::json &json, action &act);

    // プロパティ
    static action property(const char *name, nlohmann::json &json, nlohmann::json::json_pointer pointer);

    // 値
    static void value(const char *id, nlohmann::json &json);

    // 値
    static void value(nlohmann::json &json) { value("##value", json); }

    // リーフ開始
    static void begin_leaf(const char *name);

    // リーフ終了
    static void end_leaf();

    // ツリー開始
    static bool begin_tree(const char *name, nlohmann::json &json);

    // ツリー終了
    static void end_tree();

    // ファイル名
    std::string _filename;

    // ファイルパス
    std::filesystem::path _filepath;

    // ＪＳＯＮオブジェクト
    nlohmann::json _json;

    // 現在のアクション
    action _current_action;
};

} // namespace applet

#endif // APPLET_JSON_H_
