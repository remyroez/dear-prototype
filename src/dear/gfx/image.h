#ifndef DEAR_GFX_IMAGE_H_
#define DEAR_GFX_IMAGE_H_

#include <functional>

#include "sokol_fetch.h"
#include "sokol_gfx.h"
#include "imgui.h"

namespace dear::gfx {

// 画像の読み込み（同期）
sg_image load_image(const char *filename);

// 画像の読み込み（非同期）
sg_image load_image_async(const char *filename);

// sg_image を ImTextureID に変換
inline ImTextureID id(const sg_image &image) { return reinterpret_cast<ImTextureID>(image.id); }

} // namespace dear::gfx

#endif // DEAR_GFX_IMAGE_H_
