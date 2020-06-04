#include "image.h"

#include <unordered_map>

#include "stb_image.h"

bool operator==(const sfetch_handle_t &a, const sfetch_handle_t &b) {
    return a.id == b.id;
}

bool operator<(const sfetch_handle_t &a, const sfetch_handle_t &b) {
    return a.id < b.id;
}

namespace {

struct sfetch_handle_hash {
    auto operator()(const sfetch_handle_t &a) const {
        return std::hash<uint32_t>()(a.id);
    }
};

static std::unordered_map<sfetch_handle_t, dear::gfx ::load_callback, sfetch_handle_hash> s_load_callback_map;

} // namespace

namespace dear::gfx {

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

sg_image load_image(const char *filename) {
    sg_image image;

    int x, y, channels_in_file;
    int desired_channels = 4;
    if (auto *data = stbi_load(filename, &x, &y, &channels_in_file, desired_channels)) {
        // ID の確保
        image = sg_alloc_image();

        // ピクセルデータから画像生成
        sg_image_desc desc{};
        desc.width = x;
        desc.height = y;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.content.subimage[0][0].ptr = data;
        desc.content.subimage[0][0].size = x * y * c;
        sg_init_image(image, &desc);

        // ピクセルデータ開放
        stbi_image_free(data);

    } else {
        // 読み込み失敗したのでエラー画像を返す
        image = make_error_image();
    }

    return image;
}

bool load_image(const char *filename, const load_callback &callback) {
    bool succeeded = false;

    sfetch_request_t request{};
    if (auto handle = sfetch_send(&request); sfetch_handle_valid(handle)) {
        s_load_callback_map.emplace(handle, callback);
        succeeded = true;
    }

    return succeeded;
}

} // namespace dear::gfx
