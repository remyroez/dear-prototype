#ifndef DEAR_GFX_FONT_H_
#define DEAR_GFX_FONT_H_

namespace dear::gfx {

// フォント読み込み
bool load_font(const char *filename, float size_pixels = 16.f);

// フォントのビルド
void build_font();

} // namespace dear::core

#endif // DEAR_GFX_FONT_H_
