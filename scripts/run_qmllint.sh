#!/usr/bin/env bash
# Project: Crankshaft
# Utility: Run qmllint over repository QML files for local CI and debugging

set -euo pipefail

if ! command -v qmllint >/dev/null 2>&1; then
  echo "qmllint not found. Please install 'qttools6-dev-tools' or 'qttools5-dev-tools'." >&2
  exit 2
fi

EXIT_CODE=0

echo "Searching for QML files..."
# Ensure qmllint can resolve our local module (use absolute path)
REPO_ROOT="$(pwd)"
IMPORT_PATHS="${REPO_ROOT}/assets/qml"
# Do NOT include build/assets/qml in QML2_IMPORT_PATH by default.
# The build folder may contain generated or copied QML used for analysis,
# but we intentionally avoid importing from build/ during lint runs to
# prevent the linter from inspecting generated artifacts.
export QML2_IMPORT_PATH="${IMPORT_PATHS}:${QML2_IMPORT_PATH:-}"
echo "Using QML2_IMPORT_PATH=${QML2_IMPORT_PATH}"
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
