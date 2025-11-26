#!/usr/bin/env bash
# Project: Crankshaft
# This file is part of Crankshaft project.
# Copyright (C) 2025 OpenCarDev Team
#
#  Crankshaft is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  Crankshaft is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.

set -euo pipefail

usage() {
  echo "Usage: $0 [clang-tidy|cppcheck|format-check|format-apply]";
}

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

ensure_build() {
  if [ ! -d "${BUILD_DIR}" ] || [ ! -f "${BUILD_DIR}/compile_commands.json" ]; then
    cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON;
  fi
}

run_clang_tidy() {
  ensure_build;
  if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy not found. Install dev tools via VS Code task or apt.";
    exit 127;
  fi
  find "${ROOT_DIR}/src" "${ROOT_DIR}/extensions" -type f -name '*.cpp' -print0 | \
    xargs -0 -I{} clang-tidy -p "${BUILD_DIR}" {};
}

run_cppcheck() {
  if ! command -v cppcheck >/dev/null 2>&1; then
    echo "cppcheck not found. Install dev tools via VS Code task or apt.";
    exit 127;
  fi
  cppcheck --enable=all --inconclusive --std=c++17 \
    --suppress=missingIncludeSystem \
    --suppress=unknownMacro \
    "${ROOT_DIR}/src/" "${ROOT_DIR}/extensions/";
}

check_format() {
  if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found.";
    exit 127;
  fi
  find "${ROOT_DIR}/src" "${ROOT_DIR}/extensions" -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.cc' -o -name '*.h' \) -print0 | \
    xargs -0 clang-format --dry-run --Werror;
}

apply_format() {
  if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found.";
    exit 127;
  fi
  find "${ROOT_DIR}/src" "${ROOT_DIR}/extensions" -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.cc' -o -name '*.h' \) -print0 | \
    xargs -0 clang-format -i;
}

cmd=${1:-}
case "$cmd" in
  clang-tidy) run_clang_tidy ;;
  cppcheck) run_cppcheck ;;
  format-check) check_format ;;
  format-apply) apply_format ;;
  *) usage; exit 2 ;;
 esac
