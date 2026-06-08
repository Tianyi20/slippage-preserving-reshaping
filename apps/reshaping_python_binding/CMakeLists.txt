cmake_minimum_required(VERSION 3.9)
project(reshaping_python_binding)

include(eigen)

# Avoid FetchContent for pybind11 because the machine may not have GitHub access.
# Install pybind11 in the active Python first:
#   python -m pip install pybind11
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(PYBIND11_CMAKE_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")
endif()

find_package(pybind11 CONFIG REQUIRED)

# Static libraries linked into a Python extension must be position independent.
# A clean configure/build will rebuild these with -fPIC.


set(PY_PACKAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/python/slippage_reshaping")
file(MAKE_DIRECTORY "${PY_PACKAGE_OUTPUT_DIR}")

pybind11_add_module(slippage_reshaping_cpp
    pybind_module.cpp
)

target_include_directories(slippage_reshaping_cpp PRIVATE
    ${CMAKE_SOURCE_DIR}/apps/reshaping_cpp_wrapper_step1
)

target_link_libraries(slippage_reshaping_cpp PRIVATE
    reshaping_optimizer
    mesh_reshaping_lib
    Eigen3::Eigen
)

set_target_properties(slippage_reshaping_cpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PY_PACKAGE_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PY_PACKAGE_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${PY_PACKAGE_OUTPUT_DIR}"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
    "${PY_PACKAGE_OUTPUT_DIR}/__init__.py"
    COPYONLY
)
