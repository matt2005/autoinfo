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
FILES=$(find assets extensions -type f -name '*.qml' | sort)
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
