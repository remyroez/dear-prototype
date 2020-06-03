#include "image.h"

#include <unordered_map>

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

bool load_image(const char *filename, const load_callback &callback)
{
    bool succeeded = false;

    sfetch_request_t request{};
    if (auto handle = sfetch_send(&request); sfetch_handle_valid(handle)) {
        s_load_callback_map.emplace(handle, callback);
        succeeded = true;
    }

    return succeeded;
}

} // namespace dear::gfx
