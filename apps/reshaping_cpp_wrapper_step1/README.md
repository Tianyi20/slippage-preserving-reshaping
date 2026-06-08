# reshaping_cpp_wrapper_step1 fixed files

Place these files in:

```text
/home/iadc/slippage-preserving-reshaping/apps/reshaping_cpp_wrapper_step1/
```

Files:

- `ReshapingOptimizer.h`
- `ReshapingOptimizer.cpp`
- `main_with_wrapper.cpp`
- `CMakeLists.txt`

Top-level CMake reminder:

```cmake
set(RESHAPING_APP FALSE CACHE BOOL "Build 3D Reshaping GUI Application" FORCE)
set(RESHAPING_CPP_WRAPPER TRUE CACHE BOOL "Build 3D Reshaping C++ wrapper" FORCE)

if(RESHAPING_CPP_WRAPPER)
    message(STATUS "3D Reshaping C++ wrapper enabled")
    add_subdirectory("${PROJECT_SOURCE_DIR}/apps/reshaping_cpp_wrapper_step1")
endif()
```

Recommended clean build:

```bash
cd /home/iadc/slippage-preserving-reshaping
rm -rf build
mkdir build
cd build
cmake ..
cmake --build . --target reshaping_optimizer -j$(nproc)
cmake --build . --target reshaping_cpp_wrapper_demo -j$(nproc)
```

Test command:

```bash
./apps/reshaping_cpp_wrapper_step1/reshaping_cpp_wrapper_demo \
  -i /home/iadc/slippage-preserving-reshaping/models/cutlery_s26-1.obj \
  -o ~/reshaping_outputs/ \
  -e live
```
