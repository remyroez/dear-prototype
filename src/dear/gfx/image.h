#ifndef DEAR_GFX_IMAGE_H_
#define DEAR_GFX_IMAGE_H_

#include <functional>

#include "sokol_fetch.h"
#include "sokol_gfx.h"
#include "imgui.h"

namespace dear::gfx {

// 画像のチェック
bool isvalid_image(sg_image image);

// ダミー画像
sg_image dummy_image();

// sg_image を ImTextureID に変換
inline ImTextureID id(const sg_image &image) {
    return reinterpret_cast<ImTextureID>((isvalid_image(image) ? image : dummy_image()).id);
}

// 画像情報
struct image {
    // 幅
    int width = 0;

    // 高さ
    int height = 0;

    // チャンネル数
    int num_channels = 0;

    // 画像
    sg_image data { SG_INVALID_ID };

    // 変換
    operator ImTextureID() const { return dear::gfx::id(data); }
};

// 画像の読み込み（同期）
image load_image(const char *filename);

// 画像の読み込み（非同期）
sg_image load_image_async(const char *filename);

} // namespace dear::gfx

#endif // DEAR_GFX_IMAGE_H_
