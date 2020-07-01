#ifndef APPLET_BACKGROUND_H_
#define APPLET_BACKGROUND_H_

#include "dear.h"

namespace applet {

class background : public dear::applet {
    DEAR_APPLET_NOWIN("Background");

    // インストール
    virtual void install(dear::application *app) override;
    
    // 設定
    virtual void settings() override;

    // 背景画像
    dear::image _image;

    // 背景サイズ列挙型
    enum background_size {
        fixed,
        fit,
        cover,
        contain,
        custom,
    };

    // 背景サイズ
    background_size _size = background_size::fixed;

    // UV
    ImVec2 _custom_uv0 { 0.f, 0.f };
    ImVec2 _custom_uv1 { 1.f, 1.f };

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
