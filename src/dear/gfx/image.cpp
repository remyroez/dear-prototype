#include "image.h"

#include <unordered_map>

#include "stb_image.h"

// フェッチハンドルのマップキー用定義（比較）
bool operator==(const sfetch_handle_t &a, const sfetch_handle_t &b) {
    return a.id == b.id;
}

// フェッチハンドルのマップキー用定義（小なり）
bool operator<(const sfetch_handle_t &a, const sfetch_handle_t &b) {
    return a.id < b.id;
}

namespace {

// フェッチハンドルのハッシュ化関数型
struct sfetch_handle_hash {
    auto operator()(const sfetch_handle_t &a) const {
        return std::hash<uint32_t>()(a.id);
    }
};

// フェッチ中の画像マップ型
static std::unordered_map<sfetch_handle_t, sg_image, sfetch_handle_hash> s_fetching_images;

// 読み込みバッファ
// TODO: ハンドル別に動的確保
static uint8_t s_file_buffer[256 * 1024 * 1024];

// エラー画像へ初期化
void init_error_image(sg_image image) {
    uint32_t pixels[] = {
        0xFFFF00FF,
    };
    sg_image_desc desc{};
    desc.width = 1;
    desc.height = 1;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.content.subimage[0][0].ptr = pixels;
    desc.content.subimage[0][0].size = sizeof(pixels);
    sg_init_image(image, &desc);
}

// エラー画像の作成
sg_image make_error_image() {
    uint32_t pixels[] = {
        0xFFFF00FF,
    };
    sg_image_desc desc{};
    desc.width = 1;
    desc.height = 1;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.content.subimage[0][0].ptr = pixels;
    desc.content.subimage[0][0].size = sizeof(pixels);
    return sg_make_image(&desc);
}

// 画像の初期化
void init_image(sg_image image, const void *data, int x, int y, int channels_in_file) {
    if (data == nullptr) {
        // 読み込み失敗したのでエラー画像を返す
        init_error_image(image);

    } else {
        // ピクセルデータから画像生成
        sg_image_desc desc{};
        desc.width = x;
        desc.height = y;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.content.subimage[0][0].ptr = data;
        desc.content.subimage[0][0].size = x * y * channels_in_file;
        sg_init_image(image, &desc);
    }
}

// 画像読み込みコールバック
void load_cb(const sfetch_response_t *response) {
    if (!response->finished) {
        // 未完了

    } else if (auto it = s_fetching_images.find(response->handle); it != s_fetching_images.end()) {
        int x, y, channels_in_file;
        int desired_channels = 4;

        if (response->failed) {
            // 失敗
            ::init_error_image(it->second);

        } else if (!response->fetched) {
            // 未フェッチ

        } else if (auto *data = stbi_load_from_memory(
            static_cast<const stbi_uc *>(response->buffer_ptr),
            static_cast<int>(response->buffer_size),
            &x, &y, &channels_in_file, desired_channels)) {
            // 画像の作成
            ::init_image(it->second, data, x, y, channels_in_file);

            // ピクセルデータ開放
            stbi_image_free(data);

        } else {
            // その他
            ::init_error_image(it->second);
        }

        s_fetching_images.erase(it);
    }
}

} // namespace

namespace dear::gfx {

sg_image load_image(const char *filename) {
    sg_image image = sg_alloc_image();

    int x, y, channels_in_file;
    int desired_channels = 4;
    if (auto *data = stbi_load(filename, &x, &y, &channels_in_file, desired_channels)) {
        // 画像の作成
        ::init_image(image, data, x, y, channels_in_file);

        // ピクセルデータ開放
        stbi_image_free(data);

    } else {
        // 読み込み失敗したのでエラー画像を返す
        ::init_error_image(image);
    }

    return image;
}

sg_image load_image_async(const char *filename) {
    sg_image image = sg_alloc_image();

    sfetch_request_t request{};
    request.path = filename;
    request.callback = ::load_cb;
    request.buffer_ptr = ::s_file_buffer;
    request.buffer_size = sizeof(::s_file_buffer);
    //request.chunk_size = 1024 * 128;

    if (auto handle = sfetch_send(&request); sfetch_handle_valid(handle)) {
        s_fetching_images.emplace(handle, image);

    } else {
        ::init_error_image(image);
    }

    return image;
}

// ダミー画像
sg_image dummy_image() {
    static sg_image dummy = sg_alloc_image();

    if (!isvalid_image(dummy)) {
        ::init_error_image(dummy);
    }

    return dummy;
}

// 画像のチェック
bool isvalid_image(sg_image image) {
    return sg_query_image_state(image) == SG_RESOURCESTATE_VALID;
}

} // namespace dear::gfx
