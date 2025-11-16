#!/bin/bash

#
# Project: OpenAuto
# This file is part of openauto project.
# Copyright (C) 2025 OpenCarDev Team
#
#  openauto is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  openauto is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with openauto. If not, see <http://www.gnu.org/licenses/>.
#

set -e

# Configuration
BUILD_TYPE="${1:-Release}"
BUILD_TESTS="${2:-OFF}"
BUILD_DIR="${3:-build}"
PACKAGE_DEB="${4:-OFF}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Crankshaft Reborn Build ===${NC}"
echo "Build Type: ${BUILD_TYPE}"
echo "Build Tests: ${BUILD_TESTS}"
echo "Build Directory: ${BUILD_DIR}"
echo ""

# Get number of CPU cores for parallel build
if command -v nproc &> /dev/null; then
    CORES=$(nproc)
elif command -v sysctl &> /dev/null; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4
fi

echo -e "${BLUE}Using ${CORES} CPU cores for parallel build${NC}"
echo ""

# Configure CMake
echo -e "${YELLOW}Configuring CMake...${NC}"
cmake -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_TESTS="${BUILD_TESTS}" \
    -DBUILD_EXTENSIONS=ON \
    -G Ninja

# Build
echo -e "${YELLOW}Building project...${NC}"
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" --parallel "${CORES}"

# Run tests if enabled
if [ "${BUILD_TESTS}" = "ON" ]; then
    echo -e "${YELLOW}Running tests...${NC}"
    cd "${BUILD_DIR}"
    ctest --output-on-failure
    cd ..
fi

# Package as .deb if requested
if [ "${PACKAGE_DEB}" = "ON" ]; then
    echo ""
    echo -e "${YELLOW}Packaging DEB with CPack...${NC}"
    (cd "${BUILD_DIR}" && cpack -G DEB)
fi

echo ""
echo -e "${GREEN}=== Build Complete ===${NC}"
echo -e "${BLUE}Binaries are in: ${BUILD_DIR}/${NC}"
ls -lh "${BUILD_DIR}/"

if [ "${PACKAGE_DEB}" = "ON" ]; then
    echo -e "${BLUE}Generated DEB packages:${NC}"
    ls -lh "${BUILD_DIR}"/*.deb || true
fi
