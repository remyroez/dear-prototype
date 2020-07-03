
add_executable(dear_example WIN32 MACOSX_BUNDLE)
target_compile_features(dear_example PRIVATE cxx_std_17)
target_link_libraries(dear_example PRIVATE dear)

if(WIN32)
  # TODO
elseif(ANDROID)
  # TODO
elseif(APPLE)
  # TODO
elseif(UNIX AND NOT APPLE) # i.e.: Linux
  target_link_libraries(dear_example PRIVATE pthread X11 GL m dl asound)
  target_compile_definitions(dear_example PRIVATE SOKOL_GLCORE33)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

if (MSVC)
  set_target_properties(dear_example PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
  add_definitions(/bigobj)
endif()

target_sources(dear_example PRIVATE
    main.cpp
)
