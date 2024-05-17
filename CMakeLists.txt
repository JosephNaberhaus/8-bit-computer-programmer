cmake_minimum_required(VERSION 3.22)

# Enable debug prints
add_compile_definitions(DEBUG)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(programmer)
