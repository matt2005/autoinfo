#!/usr/bin/env bash
# Project: Crankshaft
# Utility: Run qmllint over repository QML files for local CI and debugging

set -euo pipefail

# CLI flags
USE_BUILD=0

print_usage() {
  cat <<EOF
Usage: $0 [options]

Options:
  -b, --use-build    Include the build/ import paths so qmllint imports generated QML from build/
  -h, --help         Show this help
EOF
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    -b|--use-build)
      USE_BUILD=1
      shift
      ;;
    -h|--help)
      print_usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      print_usage
      exit 2
      ;;
  esac
done

if ! command -v qmllint >/dev/null 2>&1; then
  echo "qmllint not found. Please install 'qttools6-dev-tools' or 'qttools5-dev-tools'." >&2
  exit 2
fi

EXIT_CODE=0

echo "Searching for QML files..."
# Ensure qmllint can resolve our local module (use absolute path)
REPO_ROOT="$(pwd)"
# Include assets/qml and the lint-only stubs module so qmllint can resolve
# project UI types. If `--use-build` is provided, prefer imports from the
# build/ tree and skip the lint-only stubs directory (they become unnecessary).
if [ "${USE_BUILD}" -eq 1 ]; then
  BUILD_IMPORTS="${REPO_ROOT}/build/assets/qml:${REPO_ROOT}/build"
  IMPORT_PATHS="${BUILD_IMPORTS}:${REPO_ROOT}/assets/qml"
  echo "Including build/ imports for qmllint (use-build enabled)."
  echo "Skipping lint stubs (tools/qml-stubs) because build/ imports are used."
else
  IMPORT_PATHS="${REPO_ROOT}/assets/qml:${REPO_ROOT}/tools/qml-stubs"
fi

export QML2_IMPORT_PATH="${IMPORT_PATHS}:${QML2_IMPORT_PATH:-}"
echo "Using QML2_IMPORT_PATH=${QML2_IMPORT_PATH}"

# Sanity check: if using build imports, ensure the build QML tree exists and
# contains a qmldir. Give a helpful message and exit if it's missing so the
# user can run a build first.
if [ "${USE_BUILD}" -eq 1 ]; then
  if [ ! -d "${REPO_ROOT}/build/assets/qml" ] || [ ! -f "${REPO_ROOT}/build/assets/qml/qmldir" ]; then
    echo "Error: build/ QML imports requested but '${REPO_ROOT}/build/assets/qml/qmldir' is missing." >&2
    echo "Run a CMake configure+build to populate the build/ QML tree, e.g.:" >&2
    echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug" >&2
    echo "  cmake --build build --config Debug -j" >&2
    exit 3
  fi
fi
# Find QML files in the source folders, explicitly excluding anything under build/
# (some workflows or tools may copy generated QML into build/ — we don't lint those.)
FILES=$(find assets extensions -type f -name '*.qml' -not -path '*/build/*' | sort)
if [ -z "${FILES}" ]; then
  echo "No QML files found."
  exit 0
fi

for f in ${FILES}; do
  echo "--- Linting: ${f} ---"
  if ! qmllint "${f}"; then
    EXIT_CODE=1
  fi
done

if [ ${EXIT_CODE} -ne 0 ]; then
  echo "qmllint found issues." >&2
fi

exit ${EXIT_CODE}
