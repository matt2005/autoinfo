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
#

set -e

# Configuration
IMAGE_NAME="crankshaft-reborn-builder"
CONTAINER_NAME="crankshaft-build"
BUILD_TYPE="${1:-Release}"
BUILD_TESTS="${2:-OFF}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Crankshaft Reborn Docker Build ===${NC}"
echo "Build Type: ${BUILD_TYPE}"
echo "Build Tests: ${BUILD_TESTS}"
echo ""

# Get the script directory (project root)
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "Project directory: ${PROJECT_DIR}"

# Detect container runtime (Docker or Podman)
CONTAINER_RUNTIME=""
if command -v docker &> /dev/null && docker ps &> /dev/null; then
    CONTAINER_RUNTIME="docker"
    echo -e "${GREEN}Using Docker${NC}"
elif command -v podman &> /dev/null; then
    CONTAINER_RUNTIME="podman"
    echo -e "${GREEN}Using Podman${NC}"
else
    echo -e "${RED}Error: Neither Docker nor Podman is available or running${NC}"
    echo -e "${YELLOW}Please install Docker or Podman and ensure it is running${NC}"
    exit 1
fi

# Build container image
echo -e "${YELLOW}Building container image...${NC}"
${CONTAINER_RUNTIME} build -t "${IMAGE_NAME}" "${PROJECT_DIR}"

# Create build directory if it doesn't exist
mkdir -p "${PROJECT_DIR}/build"

# Remove old container if exists
${CONTAINER_RUNTIME} rm -f "${CONTAINER_NAME}" 2>/dev/null || true

# Run build in container
echo -e "${YELLOW}Running build in container...${NC}"
${CONTAINER_RUNTIME} run --rm \
    --name "${CONTAINER_NAME}" \
    -v "${PROJECT_DIR}:/src" \
    -w /src \
    "${IMAGE_NAME}" \
    bash /src/build.sh "${BUILD_TYPE}" "${BUILD_TESTS}"

echo -e "${GREEN}=== Build Complete ===${NC}"
echo "Binaries are in: ${PROJECT_DIR}/build"
