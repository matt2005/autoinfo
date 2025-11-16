# Crankshaft Reborn Docker Build Script
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

param(
    [string]$BuildType = "Release",
    [string]$BuildTests = "OFF"
)

$ErrorActionPreference = "Stop"

# Configuration
$IMAGE_NAME = "crankshaft-reborn-builder"
$CONTAINER_NAME = "crankshaft-build"

Write-Host "=== Crankshaft Reborn Docker Build ===" -ForegroundColor Green
Write-Host "Build Type: $BuildType"
Write-Host "Build Tests: $BuildTests"
Write-Host ""

# Get the script directory (project root)
$PROJECT_DIR = Split-Path -Parent $PSCommandPath
Write-Host "Project directory: $PROJECT_DIR"

# Detect container runtime (Docker or Podman)
$CONTAINER_RUNTIME = $null
if (Get-Command docker -ErrorAction SilentlyContinue) {
    # Check if Docker daemon is running
    try {
        $null = docker ps 2>&1
        if ($LASTEXITCODE -eq 0) {
            $CONTAINER_RUNTIME = "docker"
            Write-Host "Using Docker" -ForegroundColor Cyan
        }
    } catch {}
}

if (-not $CONTAINER_RUNTIME -and (Get-Command podman -ErrorAction SilentlyContinue)) {
    # Check if Podman is available
    try {
        $null = podman ps 2>&1
        if ($LASTEXITCODE -eq 0) {
            $CONTAINER_RUNTIME = "podman"
            Write-Host "Using Podman" -ForegroundColor Cyan
        }
    } catch {}
}

if (-not $CONTAINER_RUNTIME) {
    Write-Host "Error: Neither Docker nor Podman is available or running." -ForegroundColor Red
    Write-Host "Please install Docker Desktop or Podman and ensure it is running." -ForegroundColor Yellow
    exit 1
}

# Build Docker image
Write-Host "Building container image..." -ForegroundColor Yellow
& $CONTAINER_RUNTIME build -t $IMAGE_NAME $PROJECT_DIR
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to build container image" -ForegroundColor Red
    exit 1
}

# Create build directory if it doesn't exist
$null = New-Item -ItemType Directory -Force -Path "$PROJECT_DIR\build"

# Remove old container if exists (silently ignore if it doesn't exist)
& $CONTAINER_RUNTIME rm -f $CONTAINER_NAME 2>&1 | Out-Null

# Run build in container
Write-Host "Running build in container..." -ForegroundColor Yellow
& $CONTAINER_RUNTIME run --rm `
    --name $CONTAINER_NAME `
    -v "${PROJECT_DIR}:/src" `
    -w /src `
    $IMAGE_NAME `
    bash -c @"
        set -e
        echo 'Configuring CMake...'
        cmake -B build \
            -DCMAKE_BUILD_TYPE=$BuildType \
            -DBUILD_TESTS=$BuildTests \
            -DBUILD_EXTENSIONS=ON \
            -G Ninja
        
        echo 'Building project...'
        cmake --build build --config $BuildType --parallel `$(nproc)
        
        if [ '$BuildTests' = 'ON' ]; then
            echo 'Running tests...'
            cd build
            ctest --output-on-failure
            cd ..
        fi
        
        echo 'Build completed successfully!'
        ls -lh build/
"@

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build failed" -ForegroundColor Red
    exit 1
}

Write-Host "=== Build Complete ===" -ForegroundColor Green
Write-Host "Binaries are in: $PROJECT_DIR\build"
