cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)

project(dear C CXX)

option(DEAR_BUILD_EXAMPLE "Build dear example program" ON)

add_subdirectory(src)

if(DEAR_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

#get_property("TARGET_SOURCE_FILES" TARGET dear PROPERTY SOURCES)
#source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})
