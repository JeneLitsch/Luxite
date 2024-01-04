cmake_minimum_required (VERSION 3.8)
     
project (voxelcast)

include_directories(./)
include_directories(./src)
include_directories(./libs/stdxx)
include_directories(./libs/stb)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(src)
# add_subdirectory(libs/sfml)