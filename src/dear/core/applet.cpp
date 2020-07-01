#include "applet.h"

#include "imgui.h"

namespace dear::core {

bool applet::begin(int flags) {
    return ImGui::Begin(name(), &_opened, flags);
}

void applet::end() {
    ImGui::End();
}

} // namespace dear::core
