# Step 2: Python binding for slippage-preserving reshaping

Place this folder here:

```text
/home/iadc/slippage-preserving-reshaping/apps/reshaping_python_binding/
```

It depends on the Step 1 wrapper target:

```text
/home/iadc/slippage-preserving-reshaping/apps/reshaping_cpp_wrapper_step1/
```

## Install pybind11

```bash
python -m pip install pybind11
```

This CMake setup avoids downloading pybind11 from GitHub. It asks Python for
pybind11's CMake directory using:

```bash
python -m pybind11 --cmakedir
```

## Patch top-level CMakeLists.txt

See:

```text
top_level_CMakeLists_python_patch.txt
```

Important: keep `RESHAPING_APP` off while developing this binding because the GUI
target has unrelated compile errors in `apps/reshaping_app/scene_viewer.cpp`.

## Clean Release build

```bash
cd /home/iadc/slippage-preserving-reshaping
rm -rf build_py
mkdir build_py
cd build_py

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target slippage_reshaping_cpp -j$(nproc)
```

## Test import

```bash
cd /home/iadc/slippage-preserving-reshaping/build_py

PYTHONPATH=$PWD/python python -c "import slippage_reshaping as sr; print(sr)"
```

## Test against your existing demo data

```bash
cd /home/iadc/slippage-preserving-reshaping/build_py

PYTHONPATH=$PWD/python python /home/iadc/slippage-preserving-reshaping/apps/reshaping_python_binding/smoke_test.py \
  -i /home/iadc/slippage-preserving-reshaping/models/cutlery_s26-1.obj \
  -e live \
  -o /home/iadc/reshaping_outputs_py
```

## Python API: array input path

```python
import slippage_reshaping as sr

result = sr.optimize(
    V,                 # numpy array, shape (#V, 3)
    F,                 # numpy int array, shape (#F, 3)
    face_k1,           # numpy array, shape (#F,)
    face_k2,           # numpy array, shape (#F,)
    constraint_ids,    # list[int]
    target_positions,  # numpy array, shape (N, 3)
)

V_opt = result.vertices
F_out = result.faces
```

## Python API: existing demo-file compatibility path

```python
import slippage_reshaping as sr

options = sr.Options(max_iters=100, input_name="cutlery_live_py")

result = sr.optimize_from_edit_file(
    "/home/iadc/slippage-preserving-reshaping/models/cutlery_s26-1.obj",
    "live",
    options,
)

sr.save_obj(result.vertices, result.faces, "/home/iadc/reshaping_outputs_py", "cutlery_live_py", "output")
```
