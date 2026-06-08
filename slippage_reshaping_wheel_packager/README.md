# Local wheel packager for `slippage_reshaping`

This folder packages the already-built pybind11 `.so` into a local binary wheel.

It does **not** rebuild C++ code. It copies from:

```text
/home/iadc/slippage-preserving-reshaping/build_py/python/slippage_reshaping/
```

Expected files there:

```text
__init__.py
slippage_reshaping_cpp.cpython-39-x86_64-linux-gnu.so
```

## Build wheel

```bash
cd /path/to/slippage_reshaping_wheel_packager
bash prepare_and_build_wheel.sh
```

If your build directory is different:

```bash
BUILD_DIR=/home/iadc/slippage-preserving-reshaping/build_py bash prepare_and_build_wheel.sh
```

## Install wheel

```bash
pip install dist/slippage_reshaping-0.1.0-*.whl
```

## Test

```bash
python -c "import slippage_reshaping as sr; print(sr)"
```

## Editable-ish local install

For day-to-day development, it is usually easier to keep using:

```bash
export PYTHONPATH=/home/iadc/slippage-preserving-reshaping/build_py/python:$PYTHONPATH
```

Then rebuild the C++ extension with CMake whenever C++ changes.
