#!/bin/bash
#
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

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
BUILD_TYPE="${1:-Release}"

# Best-effort install of Bluetooth dependencies if missing (Debian-based hosts).
ensure_packages() {
    if ! command -v apt-get >/dev/null 2>&1; then
        return 0
    fi
    local pkgs=(bluez bluez-tools libbluetooth-dev qt6-connectivity-dev)
    local missing=()
    for p in "${pkgs[@]}"; do
        if ! dpkg -s "$p" >/dev/null 2>&1; then
            missing+=("$p")
        fi
    done
    if [ ${#missing[@]} -eq 0 ]; then
        return 0
    fi
    echo "Installing missing Bluetooth packages: ${missing[*]}" || true
    if [ "$(id -u)" -ne 0 ]; then
        if command -v sudo >/dev/null 2>&1; then
            sudo apt-get update && sudo apt-get install -y "${missing[@]}" || true
        else
            echo "(No sudo available; please install manually: ${missing[*]})" || true
        fi
    else
        apt-get update && apt-get install -y "${missing[@]}" || true
    fi
}

ensure_packages

echo "Building Crankshaft Reborn..."
echo "Build type: ${BUILD_TYPE}"

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_TESTS=ON \
    -DBUILD_EXTENSIONS=ON

# Build
cmake --build . --config "${BUILD_TYPE}" -j$(nproc)

echo "Build completed successfully!"
echo "Binary location: ${BUILD_DIR}/CrankshaftReborn"
