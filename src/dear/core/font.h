#ifndef DEAR_CORE_FONT_H_
#define DEAR_CORE_FONT_H_

namespace dear::core {

// フォント読み込み
bool load_font(const char *filename, float size_pixels = 16.f);

// フォントのビルド
void build_font();

} // namespace dear::core

#endif // DEAR_CORE_FONT_H_
