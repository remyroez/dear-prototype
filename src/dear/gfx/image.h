#ifndef DEAR_GFX_IMAGE_H_
#define DEAR_GFX_IMAGE_H_

#include <functional>

#include "sokol_fetch.h"
#include "sokol_gfx.h"
#include "imgui.h"

namespace dear::gfx {

// 画像情報
struct image {
    // ダミー
    static const image &dummy();

    // 幅
    int width = 0;

    // 高さ
    int height = 0;

    // チャンネル数
    int num_channels = 0;

    // 画像
    sg_image data { SG_INVALID_ID };

    // 変換
    operator ImTextureID() const { return reinterpret_cast<ImTextureID>((static_cast<bool>(*this) ? data.id : image::dummy().data.id)); }

    // 有効判定
    operator bool() const { return sg_query_image_state(data) == SG_RESOURCESTATE_VALID; }
};

// 画像の読み込み（同期）
bool load_image(const char *filename, image &img);

// 画像の読み込み（非同期）
bool load_image_async(const char *filename, image &img);

} // namespace dear::gfx

#endif // DEAR_GFX_IMAGE_H_
