#ifndef DEAR_GFX_IMAGE_H_
#define DEAR_GFX_IMAGE_H_

#include <functional>

#include "sokol_fetch.h"

namespace dear::gfx {

using load_callback = std::function<void(const sfetch_response_t*)>;

bool load_image(const char *filename, const load_callback &callback);

} // namespace dear::gfx

#endif // DEAR_GFX_IMAGE_H_
