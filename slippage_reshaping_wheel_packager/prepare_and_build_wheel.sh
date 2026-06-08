#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="${REPO_ROOT:-/home/iadc/slippage-preserving-reshaping}"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build_py}"
PACKAGE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SRC_PKG="${BUILD_DIR}/python/slippage_reshaping"
DST_PKG="${PACKAGE_DIR}/slippage_reshaping"

if [ ! -d "${SRC_PKG}" ]; then
  echo "ERROR: Cannot find built package: ${SRC_PKG}"
  echo "Build the binding first:"
  echo "  cd ${BUILD_DIR}"
  echo "  cmake --build . --target slippage_reshaping_cpp -j\$(nproc)"
  exit 1
fi

rm -rf "${DST_PKG}"
mkdir -p "${DST_PKG}"

cp "${SRC_PKG}/__init__.py" "${DST_PKG}/"
cp "${SRC_PKG}"/slippage_reshaping_cpp*.so "${DST_PKG}/"

python -m pip install -U build wheel setuptools
python -m build --wheel "${PACKAGE_DIR}"

echo
echo "Wheel generated under:"
echo "  ${PACKAGE_DIR}/dist/"
ls -lh "${PACKAGE_DIR}/dist/"
