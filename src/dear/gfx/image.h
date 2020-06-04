#ifndef DEAR_GFX_IMAGE_H_
#define DEAR_GFX_IMAGE_H_

#include <functional>

#include "sokol_fetch.h"
#include "sokol_gfx.h"
#include "imgui.h"

namespace dear::gfx {

using load_callback = std::function<void(const sg_image)>;

// sg_image を ImTextureID に変換
inline ImTextureID id(const sg_image &image) { return reinterpret_cast<ImTextureID>(image.id); }

// エラー画像の作成
sg_image make_error_image();

// 画像の読み込み
sg_image load_image(const char *filename);

bool load_image(const char *filename, const load_callback &callback);

} // namespace dear::gfx

#endif // DEAR_GFX_IMAGE_H_
