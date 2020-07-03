
add_library(dear)
target_compile_features(dear PRIVATE cxx_std_17)

if(WIN32)
  # TODO
elseif(ANDROID)
  # TODO
elseif(APPLE)
  # TODO
elseif(UNIX AND NOT APPLE) # i.e.: Linux
  target_compile_definitions(dear PRIVATE SOKOL_GLCORE33)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

if (MSVC)
  add_definitions(/bigobj)
endif()

target_include_directories(dear PUBLIC .)

add_subdirectory(applet)
add_subdirectory(dear)
add_subdirectory(thirdparty)
