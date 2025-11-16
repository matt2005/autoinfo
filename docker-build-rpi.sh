#!/bin/bash

# * Project: Crankshaft Reborn
# * This file is part of crankshaft_reborn project.
# * Copyright (C) 2025 OpenCarDev Team
# *
# *  crankshaft_reborn is free software: you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation; either version 3 of the License, or
# *  (at your option) any later version.
# *
# *  crankshaft_reborn is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with crankshaft_reborn. If not, see <http://www.gnu.org/licenses/>.

set -e
set -u

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}================================================${NC}"
    echo -e "${BLUE}  Crankshaft Reborn Raspberry Pi Build Script${NC}"
    echo -e "${BLUE}================================================${NC}"
}

print_step() {
    echo -e "${YELLOW}🔄 $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

show_usage() {
    echo "Crankshaft Reborn Raspberry Pi Docker/Podman Build Script"
    echo
    echo "Usage: $0 [ARCHITECTURE] [BUILD_TYPE] [OPTIONS]"
    echo
    echo "ARCHITECTURE:"
    echo "  armhf       Build for ARM 32-bit (Raspberry Pi 3) (default)"
    echo "  arm64       Build for ARM 64-bit (Raspberry Pi 4/5)"
    echo "  all         Build for all architectures"
    echo
    echo "BUILD_TYPE:"
    echo "  release     Release build (default)"
    echo "  debug       Debug build"
    echo
    echo "OPTIONS:"
    echo "  --clean     Clean build output directory before building"
    echo "  --no-cache  Don't use build cache"
    echo "  --podman    Use Podman instead of Docker"
    echo "  --help      Show this help message"
    echo
    echo "Examples:"
    echo "  $0 armhf release         # Build ARMv7 release"
    echo "  $0 arm64 debug --clean   # Clean ARM64 debug build"
    echo "  $0 all release --podman  # Build all architectures using Podman"
    echo
}

# Default values
ARCH="armhf"
BUILD_TYPE="release"
CLEAN=false
NO_CACHE=""
CONTAINER_CMD="docker"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        armhf|arm64|all)
            ARCH=$1
            shift
            ;;
        release|debug|Release|Debug)
            BUILD_TYPE=$(echo "$1" | tr '[:upper:]' '[:lower:]')
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --no-cache)
            NO_CACHE="--no-cache"
            shift
            ;;
        --podman)
            CONTAINER_CMD="podman"
            shift
            ;;
        --help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

print_header

# Detect container runtime
if ! command -v ${CONTAINER_CMD} &> /dev/null; then
    if [ "${CONTAINER_CMD}" = "podman" ]; then
        print_error "Podman not found. Please install Podman or use Docker."
        exit 1
    else
        print_step "Docker not found, checking for Podman..."
        if command -v podman &> /dev/null; then
            CONTAINER_CMD="podman"
            print_success "Using Podman"
        else
            print_error "Neither Docker nor Podman found. Please install one."
            exit 1
        fi
    fi
else
    print_success "Using ${CONTAINER_CMD}"
fi

# Check for buildx support (Docker only)
if [ "${CONTAINER_CMD}" = "docker" ]; then
    if ! docker buildx version &> /dev/null; then
        print_error "Docker Buildx not found. Please install Docker Buildx."
        exit 1
    fi
    print_success "Docker Buildx available"
fi

# Setup QEMU for cross-platform emulation
print_step "Setting up QEMU for ARM emulation..."
if [ "${CONTAINER_CMD}" = "docker" ]; then
    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes > /dev/null 2>&1
else
    # Podman uses system QEMU
    if ! command -v qemu-arm-static &> /dev/null; then
        print_error "QEMU not found. Please install qemu-user-static."
        exit 1
    fi
fi
print_success "QEMU setup complete"

build_architecture() {
    local arch=$1
    local build_cache=$2
    
    print_step "Building for ${arch} (${BUILD_TYPE})..."
    
    # Map architecture to platform
    local platform="linux/${arch}"
    if [ "${arch}" = "armhf" ]; then
        platform="linux/arm/v7"
    fi
    
    # Prepare build arguments
    local build_args=""
    if [ -n "$build_cache" ]; then
        build_args="$build_cache"
    fi
    
    # Build using Docker Buildx or Podman
    if [ "${CONTAINER_CMD}" = "docker" ]; then
        docker buildx build \
            $build_args \
            --platform ${platform} \
            --build-arg TARGET_ARCH=${arch} \
            --build-arg BUILD_TYPE=${BUILD_TYPE} \
            --tag crankshaft-reborn-${arch}:${BUILD_TYPE} \
            --load \
            -f Dockerfile.rpi \
            .
        
        # Extract binaries
        local container_id=$(docker create crankshaft-reborn-${arch}:${BUILD_TYPE})
        docker cp ${container_id}:/output/. ./build-output-${arch}/
        docker rm ${container_id} > /dev/null
    else
        # Podman build
        podman build \
            $build_args \
            --arch ${arch} \
            --build-arg TARGET_ARCH=${arch} \
            --build-arg BUILD_TYPE=${BUILD_TYPE} \
            --tag crankshaft-reborn-${arch}:${BUILD_TYPE} \
            -f Dockerfile.rpi \
            .
        
        # Extract binaries
        local container_id=$(podman create crankshaft-reborn-${arch}:${BUILD_TYPE})
        podman cp ${container_id}:/output/. ./build-output-${arch}/
        podman rm ${container_id} > /dev/null
    fi
    
    print_success "Build completed for ${arch}"
}

# Clean output directory if requested
if [ "$CLEAN" = true ]; then
    print_step "Cleaning build output directories..."
    rm -rf build-output-*
fi

# Create output directories
mkdir -p build-output-armhf build-output-arm64

# Build for specified architecture(s)
case $ARCH in
    armhf|arm64)
        build_architecture $ARCH "$NO_CACHE"
        ;;
    all)
        print_step "Building for all architectures..."
        build_architecture "armhf" "$NO_CACHE"
        build_architecture "arm64" "$NO_CACHE"
        print_success "All architectures built successfully"
        ;;
    *)
        print_error "Unknown architecture: $ARCH"
        show_usage
        exit 1
        ;;
esac

print_step "Build results:"
if [ -d "build-output-armhf" ]; then
    echo
    echo "ARMv7 (armhf) binaries:"
    ls -lh build-output-armhf/ 2>/dev/null || echo "  (no files)"
fi
if [ -d "build-output-arm64" ]; then
    echo
    echo "ARM64 binaries:"
    ls -lh build-output-arm64/ 2>/dev/null || echo "  (no files)"
fi

echo
print_success "Build complete!"
