cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)

project(dear C CXX)

add_executable(dear WIN32 MACOSX_BUNDLE)
target_compile_features(dear PRIVATE cxx_std_17)
target_include_directories(dear PRIVATE include)
target_link_directories(dear PRIVATE lib)

option(DEAR_BUILD_EXAMPLE ON)

if(WIN32)
  # TODO
elseif(ANDROID)
  # TODO
elseif(APPLE)
  # TODO
elseif(UNIX AND NOT APPLE) # i.e.: Linux
  target_link_libraries(dear PRIVATE pthread X11 GL m dl asound)
  target_compile_definitions(dear PRIVATE SOKOL_GLCORE33)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

if (MSVC)
  set_target_properties(dear PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
  add_definitions(/bigobj)
endif()

add_subdirectory(src)

if(DEAR_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

get_property("TARGET_SOURCE_FILES" TARGET dear PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})
