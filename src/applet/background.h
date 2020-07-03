#ifndef APPLET_BACKGROUND_H_
#define APPLET_BACKGROUND_H_

#include <string>

#include "dear.h"

namespace applet {

class background : public dear::applet {
    DEAR_APPLET_NOWIN("Background");

    // インストール
    virtual void install(dear::application *app) override;

    // 背景色の描画
    void render_background_color();

    // 背景画像の描画
    void render_background_image();

    // 背景画像のＵＶ計算
    void calc_image_uv(const ImVec2 &rect, ImVec2 &uv0, ImVec2 &uv1);
    
    // 設定
    virtual void settings() override;

    // 使用するかどうか
    bool _enable_color = true;
    bool _enable_image = true;

    // ファイルパス
    std::string _filepath;

    // 背景画像
    dear::image _image;

    // 画像色
    ImColor _image_color { IM_COL32_WHITE };

    // 背景サイズ列挙型
    enum background_size {
        fixed,
        fit,
        cover,
        contain,
        custom,
    };

    // 背景サイズ
    background_size _size = background_size::cover;

    // UV
    ImVec2 _custom_uv0 { 0.f, 0.f };
    ImVec2 _custom_uv1 { 1.f, 1.f };

    // 背景色
    ImColor _color_top_left { IM_COL32_BLACK };
    ImColor _color_top_right { IM_COL32_BLACK };
    ImColor _color_bottom_left { IM_COL32_BLACK };
    ImColor _color_bottom_right { IM_COL32_BLACK };

    // マルチカラー
    bool _multi_color = false;

public:
    // 背景画像の読み込み
    void load_background_image(const char *filename);

    // 背景画像のラップの設定
    void set_background_image_wrap(sg_wrap wrap_u = _SG_WRAP_DEFAULT, sg_wrap wrap_v = _SG_WRAP_DEFAULT) {
        _image.wrap_u = wrap_u;
        _image.wrap_v = wrap_v;
    }

    // 背景サイズの設定
    void set_background_size(background_size size) {
        _size = size;
    }

    // 背景サイズの設定 (UV指定)
    void set_background_size(const ImVec2 &uv0 = ImVec2(0.f, 0.f), const ImVec2 &uv1 = ImVec2(1.f, 1.f)) {
        set_background_size(background_size::custom);
        _custom_uv0 = uv0;
        _custom_uv1 = uv1;
    }
};

} // namespace applet

#endif // APPLET_BACKGROUND_H_
