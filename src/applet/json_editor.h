#ifndef APPLET_JSON_H_
#define APPLET_JSON_H_

#include "dear.h"
#include "json.hpp"

#include <string>

namespace applet {

class json_editor : public dear::applet {
    // 名前
    virtual const char *name() override {
        return "json_editor";
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
            insert,
            clear,
        };

        // モード
        mode_t mode = mode_t::none;

        // 対象のＪＳＯＮオブジェクト
        nlohmann::json *target = nullptr;

        // リセット
        void reset() {
            mode = mode_t::none;
            target = nullptr;
        }

        // アクションがあるかどうか
        operator bool() const { return mode != mode_t::none; }
    };

    // プロパティ
    static action property(const char *name, nlohmann::json &json);

    // リーフ開始
    static void begin_leaf(const char *name);

    // リーフ終了
    static void end_leaf();

    // ツリー開始
    static bool begin_tree(const char *name, const char *text, int size, action &act);

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
