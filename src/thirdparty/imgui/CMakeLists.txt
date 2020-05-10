
target_include_directories(dear PRIVATE .)

target_sources(dear PRIVATE
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui.cpp
)

add_subdirectory(misc)
