cmake_minimum_required(VERSION 3.25)

project(Calorimeter VERSION 1.0 DESCRIPTION "Timing a function.")
include_directories("include/")
add_executable(Calorimeter calorimeter.cpp)
add_custom_command(TARGET Calorimeter POST_BUILD COMMAND "${CMAKE_SOURCE_DIR}/objdump_wrapper.sh" "$<TARGET_FILE:Calorimeter>" "${CMAKE_SOURCE_DIR}/Calorimeter.obj")