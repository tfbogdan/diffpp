cmake_minimum_required(VERSION 3.17)

project(diffpp)

add_library(diffpp-headers INTERFACE)
add_library(diffpp INTERFACE)

target_compile_features(diffpp INTERFACE cxx_std_17)
target_compile_features(diffpp-headers INTERFACE cxx_std_17)

add_subdirectory(include)
add_subdirectory(test)
