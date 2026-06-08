cmake_minimum_required(VERSION 3.9)
project(reshaping_cpp_wrapper_step1)

include(FetchContent)
include(cli11)
include(eigen)

# Core wrapper library. This is the target that the future pybind11 module
# should link against.
add_library(reshaping_optimizer STATIC
    ReshapingOptimizer.h
    ReshapingOptimizer.cpp
)

target_include_directories(reshaping_optimizer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(reshaping_optimizer PUBLIC
    mesh_reshaping_lib
    Eigen3::Eigen
)

# Optional command-line executable for testing the wrapper with the existing
# .deform/.fk demo data path.
#
# It reuses load_input_data.cpp/h from apps/reshaping_demo, so we do not duplicate
# the existing file-loading code.
add_executable(reshaping_cpp_wrapper_demo
    main_with_wrapper.cpp
    ${CMAKE_SOURCE_DIR}/apps/reshaping_demo/load_input_data.cpp
)

target_include_directories(reshaping_cpp_wrapper_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/apps/reshaping_demo
)

target_link_libraries(reshaping_cpp_wrapper_demo PRIVATE
    reshaping_optimizer
    mesh_reshaping_lib
    Eigen3::Eigen
    CLI11::CLI11
)
