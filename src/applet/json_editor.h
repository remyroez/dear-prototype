#ifndef APPLET_JSON_H_
#define APPLET_JSON_H_

#include "dear.h"
#include "json.hpp"

#include <string>

namespace applet {

class json_editor : public dear::applet {
    // 名前
    virtual const char *name() override {
        return "JSON Editor";
    }

    // インストール
    virtual void install(dear::application *app) override;

    // フレーム経過
    void frame(double delta_time);

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

    // ＪＳＯＮオブジェクト
    nlohmann::json _json;

    // 現在のアクション
    action _current_action;
};

} // namespace applet

#endif // APPLET_JSON_H_
