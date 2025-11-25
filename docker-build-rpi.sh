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

ARCH="${1:-armhf}"
BUILD_TYPE="${2:-release}"
CLEAN="${CLEAN:-0}"
NOCACHE="${NOCACHE:-0}"
USE_PODMAN="${USE_PODMAN:-0}"

case "${ARCH}" in
  armhf|arm64|all) ;;
  *) echo "Invalid architecture: ${ARCH}"; exit 1;;
esac

container_cmd="docker"
if [ "${USE_PODMAN}" = "1" ]; then container_cmd="podman"; fi
if ! command -v "${container_cmd}" >/dev/null 2>&1; then
  if [ "${container_cmd}" = "docker" ] && command -v podman >/dev/null 2>&1; then
    container_cmd="podman"
  else
    echo "Neither Docker nor Podman found. Please install one."; exit 1
  fi
fi

if [ "${container_cmd}" = "docker" ]; then
  if ! docker buildx version >/dev/null 2>&1; then
    echo "Docker Buildx not found. Please install Docker Buildx."; exit 1
  fi
  echo "Setting up QEMU for ARM emulation..."
  docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
fi

if [ "${CLEAN}" = "1" ]; then
  echo "Cleaning build output directories..."
  rm -rf build-output-armhf build-output-arm64 || true
fi
mkdir -p build-output-armhf build-output-arm64

cache_arg=""
if [ "${NOCACHE}" = "1" ]; then cache_arg="--no-cache"; fi

build_one() {
  local arch="$1"
  local platform="linux/${arch}"
  [ "${arch}" = "armhf" ] && platform="linux/arm/v7"

  local tag="crankshaft-reborn-${arch}:${BUILD_TYPE}"
  if [ "${container_cmd}" = "docker" ]; then
    docker buildx build \
      --platform "${platform}" \
      --build-arg TARGET_ARCH="${arch}" \
      --build-arg BUILD_TYPE="${BUILD_TYPE}" \
      --tag "${tag}" \
      --load \
      -f Dockerfile.rpi \
      ${cache_arg} \
      .
    local cid
    cid=$(docker create "${tag}")
    docker cp "${cid}:/output/." "./build-output-${arch}/"
    docker rm "${cid}" >/dev/null
  else
    podman build \
      --arch "${arch}" \
      --build-arg TARGET_ARCH="${arch}" \
      --build-arg BUILD_TYPE="${BUILD_TYPE}" \
      --tag "${tag}" \
      -f Dockerfile.rpi \
      ${cache_arg} \
      .
    local cid
    cid=$(podman create "${tag}")
    podman cp "${cid}:/output/." "./build-output-${arch}/"
    podman rm "${cid}" >/dev/null
  fi
}

if [ "${ARCH}" = "all" ]; then
  build_one armhf
  build_one arm64
else
  build_one "${ARCH}"
fi

echo "Build complete!"
ls -la build-output-armhf || true
ls -la build-output-arm64 || true
