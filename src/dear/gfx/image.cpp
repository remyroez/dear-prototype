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

static const int s_desired_channels = 4;

// フェッチハンドルのハッシュ化関数型
struct sfetch_handle_hash {
    auto operator()(const sfetch_handle_t &a) const {
        return std::hash<uint32_t>()(a.id);
    }
};

// フェッチ中の画像マップ型
static std::unordered_map<sfetch_handle_t, dear::gfx::image *, sfetch_handle_hash> s_fetching_images;

// 読み込みバッファ
// TODO: ハンドル別に動的確保
static uint8_t s_file_buffer[256 * 1024 * 1024];

// エラー画像へ初期化
void init_error_image(dear::gfx::image &image) {
    image.num_channels = 1;
    image.width = 1;
    image.height = 1;

    uint32_t pixels[] = {
        0xFFFF00FF,
    };
    sg_image_desc desc{};
    desc.width = image.width;
    desc.height = image.height;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.content.subimage[0][0].ptr = pixels;
    desc.content.subimage[0][0].size = sizeof(pixels);
    sg_init_image(image.data, &desc);
}

// 画像の初期化
void init_image(dear::gfx::image &image, const void *data) {
    sg_image_desc desc{};
    desc.width = image.width;
    desc.height = image.height;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.min_filter = SG_FILTER_LINEAR;
    desc.mag_filter = SG_FILTER_LINEAR;
    desc.content.subimage[0][0].ptr = data;
    desc.content.subimage[0][0].size = image.width * image.height * image.num_channels;
    sg_init_image(image.data, &desc);
}

// 画像読み込みコールバック
void load_cb(const sfetch_response_t *response) {
    if (!response->finished) {
        // 未完了

    } else if (auto it = s_fetching_images.find(response->handle); it != s_fetching_images.end()) {
        auto &img = *it->second;

        const int desired_channels = 4;

        if (response->failed) {
            // 失敗
            ::init_error_image(img);

        } else if (!response->fetched) {
            // 未フェッチ

        } else if (auto *data = stbi_load_from_memory(
            static_cast<const stbi_uc *>(response->buffer_ptr),
            static_cast<int>(response->buffer_size),
            &img.width, &img.height, &img.num_channels, s_desired_channels)
        ) {
            // 画像の作成
            ::init_image(img, data);

            // ピクセルデータ開放
            stbi_image_free(data);

        } else {
            // その他
            ::init_error_image(img);
        }

        s_fetching_images.erase(it);
    }
}

} // namespace

namespace dear::gfx {

bool load_image(const char *filename, image &img) {
    bool succeeded = false;

    // 画像IDの確保
    img.data = sg_alloc_image();

    if (auto *data = stbi_load(filename, &img.width, &img.height, &img.num_channels, s_desired_channels)) {
        // 画像の作成
        ::init_image(img, data);

        // ピクセルデータ開放
        stbi_image_free(data);

        succeeded = true;

    } else {
        // 読み込み失敗したのでエラー画像を返す
        ::init_error_image(img);
    }

    return succeeded;
}

bool load_image_async(const char *filename, image &img) {
    bool succeeded = false;

    // 画像IDの確保
    img.data = sg_alloc_image();

    // リクエスト
    sfetch_request_t request{};
    request.path = filename;
    request.callback = ::load_cb;
    request.buffer_ptr = ::s_file_buffer;
    request.buffer_size = sizeof(::s_file_buffer);
    //request.chunk_size = 1024 * 128;

    if (auto handle = sfetch_send(&request); sfetch_handle_valid(handle)) {
        s_fetching_images.emplace(handle, &img);
        succeeded = true;

    } else {
        ::init_error_image(img);
    }

    return succeeded;
}

// ダミー画像
const image &image::dummy() {
    static image dummy;
    
    if (dummy.data.id == SG_INVALID_ID) {
        dummy.data = sg_alloc_image();
    }

    if (!dummy) {
        ::init_error_image(dummy);
    }

    return dummy;
}

void calc_uvs_fixed(float image_width, float image_height, float rect_width, float rect_height, ImVec2 &uv0, ImVec2 &uv1) {
    const auto iw = (image_width > 0 ? image_width : 1.f);
    const auto ih = (image_height > 0 ? image_height : 1.f);
    uv0.x = 0.f;
    uv0.y = 0.f;
    uv1.x = rect_width / iw;
    uv1.y = rect_height / ih;
}

void calc_uvs_cover(float image_width, float image_height, float rect_width, float rect_height, ImVec2 &uv0, ImVec2 &uv1) {
    const auto width = rect_width;
    const auto height = rect_height;
    const auto iw = (image_width > 0 ? image_width : 1.f);
    const auto ih = (image_height > 0 ? image_height : 1.f);
    const auto aspect_w = (width / height);
    const auto aspect_h = (height / width);
    const auto aspect_iw = (iw / ih);
    const auto aspect_ih = (ih / iw);
    const auto ratio_w = (iw / width);
    const auto ratio_h = (ih / height);
    uv0.x = 0.f;
    uv0.y = 0.f;
    if (aspect_w > aspect_iw) {
        uv1.x = 1.f;
        uv1.y = ratio_w / ratio_h;
        uv0.y = -(uv1.y - 1.f) * .5f;
        uv1.y += uv0.y;

    } else {
        uv1.x = ratio_h / ratio_w;
        uv1.y = 1.f;
        uv0.x = -(uv1.x - 1.f) * .5f;
        uv1.x += uv0.x;
    }
}

void calc_uvs_contain(float image_width, float image_height, float rect_width, float rect_height, ImVec2 &uv0, ImVec2 &uv1) {
    const auto width = rect_width;
    const auto height = rect_height;
    const auto iw = (image_width > 0 ? image_width : 1.f);
    const auto ih = (image_height > 0 ? image_height : 1.f);
    const auto aspect_w = (width / height);
    const auto aspect_h = (height / width);
    const auto aspect_iw = (iw / ih);
    const auto aspect_ih = (ih / iw);
    const auto ratio_w = (iw / width);
    const auto ratio_h = (ih / height);
    uv0.x = 0.f;
    uv0.y = 0.f;
    if (aspect_w > aspect_iw) {
        uv1.x = ratio_h / ratio_w;
        uv1.y = 1.f;
        uv0.x = -(uv1.x - 1.f) * .5f;
        uv1.x += uv0.x;

    } else {
        uv1.x = 1.f;
        uv1.y = ratio_w / ratio_h;
        uv0.y = -(uv1.y - 1.f) * .5f;
        uv1.y += uv0.y;
    }
}

} // namespace dear::gfx
