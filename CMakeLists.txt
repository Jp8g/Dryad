cmake_minimum_required(VERSION 3.10)

project(DRYAD C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

add_library(dryad STATIC src/dryad.c)

target_link_libraries(dryad PRIVATE asound)

target_include_directories(dryad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)