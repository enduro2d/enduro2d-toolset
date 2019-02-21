cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(enduro2d-toolset)

option(E2D_BUILD_MODEL_CONVERTER "Build model converter" ON)
if(E2D_BUILD_MODEL_CONVERTER)
    add_subdirectory(model_converter)
endif()
