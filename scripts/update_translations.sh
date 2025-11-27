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

#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# update_translations.sh
# Purpose: Extract and compile Qt translation files for core and extensions.
# British English (en_GB) is the default base locale.
# Usage:
#   ./scripts/update_translations.sh            # update .ts and compile .qm
#   LOCALE=fr ./scripts/update_translations.sh  # generate/update for a different locale (experimental)
#   ./scripts/update_translations.sh extract    # only run lupdate (do not compile)
#   ./scripts/update_translations.sh compile    # only run lrelease (assumes .ts exists)
#   VERBOSE=1 ./scripts/update_translations.sh  # enable verbose output

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
LOCALE="${LOCALE:-en_GB}"
MODE="${1:-all}"
TS_DIR_CORE="${ROOT_DIR}/i18n"
EXT_DIR="${ROOT_DIR}/extensions"
LUPDATE_BIN="$(command -v lupdate || true)"
LRELEASE_BIN="$(command -v lrelease || true)"

if [[ -z "${LUPDATE_BIN}" || -z "${LRELEASE_BIN}" ]]; then
  echo "Error: lupdate or lrelease not found. Install qttools6-dev-tools (Qt6) or qttools5-dev-tools (Qt5)." >&2
  exit 1
fi

mkdir -p "${TS_DIR_CORE}"
CORE_TS_FILE="${TS_DIR_CORE}/core_${LOCALE}.ts"

# Discover extensions (directories containing at least one .cpp/.qml)
mapfile -t EXTENSIONS < <(find "${EXT_DIR}" -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | sort)

# Filter out empty or build artefact directories
VALID_EXTENSIONS=()
for ext in "${EXTENSIONS[@]}"; do
  if find "${EXT_DIR}/${ext}" -type f \( -name '*.cpp' -o -name '*.qml' \) | grep -q .; then
    VALID_EXTENSIONS+=("${ext}")
  fi
done

[[ "${VERBOSE:-0}" == "1" ]] && echo "Detected extensions: ${VALID_EXTENSIONS[*]}"

update_core_ts() {
  echo "==> Updating core translations (${CORE_TS_FILE})"
  "${LUPDATE_BIN}" \
    "${ROOT_DIR}/src" \
    "${ROOT_DIR}/assets/qml" \
    -recursive -locations relative -no-obsolete \
    -ts "${CORE_TS_FILE}" || {
      echo "Error: lupdate failed for core" >&2; return 1; }
}

update_extension_ts() {
  local ext="$1"
  local ext_dir="${EXT_DIR}/${ext}"
  local i18n_dir="${ext_dir}/i18n"
  mkdir -p "${i18n_dir}"
  local ts_file="${i18n_dir}/${ext}_${LOCALE}.ts"
  echo "==> Updating extension '${ext}' translations (${ts_file})"
  "${LUPDATE_BIN}" \
    "${ext_dir}" \
    -recursive -locations relative -no-obsolete \
    -ts "${ts_file}" || {
      echo "Warning: lupdate failed for extension ${ext}" >&2; }
}

compile_ts_files() {
  echo "==> Compiling .ts files to .qm"
  local ts_files=("${CORE_TS_FILE}")
  for ext in "${VALID_EXTENSIONS[@]}"; do
    local ts_path="${EXT_DIR}/${ext}/i18n/${ext}_${LOCALE}.ts"
    [[ -f "${ts_path}" ]] && ts_files+=("${ts_path}")
  done
  for ts in "${ts_files[@]}"; do
    local qm_target="${ts%.ts}.qm"
    echo "    lrelease ${ts} -> ${qm_target}"
    "${LRELEASE_BIN}" "${ts}" -qm "${qm_target}" || {
      echo "Error: lrelease failed for ${ts}" >&2; return 1; }
  done
}

case "${MODE}" in
  extract)
    update_core_ts
    for ext in "${VALID_EXTENSIONS[@]}"; do update_extension_ts "${ext}"; done
    ;;
  compile)
    compile_ts_files
    ;;
  all)
    update_core_ts
    for ext in "${VALID_EXTENSIONS[@]}"; do update_extension_ts "${ext}"; done
    compile_ts_files
    ;;
  *)
    echo "Usage: $0 [all|extract|compile]" >&2
    exit 2
    ;;
 esac

 echo "Translation update complete (mode=${MODE}, locale=${LOCALE})"
