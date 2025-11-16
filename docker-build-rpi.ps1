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

param(
    [Parameter(Position=0)]
    [ValidateSet('armhf', 'arm64', 'all')]
    [string]$Architecture = 'armhf',
    
    [Parameter(Position=1)]
    [ValidateSet('release', 'debug')]
    [string]$BuildType = 'release',
    
    [switch]$Clean,
    [switch]$NoCache,
    [switch]$Podman
)

$ErrorActionPreference = 'Stop'

function Write-Header {
    Write-Host "================================================" -ForegroundColor Blue
    Write-Host "  Crankshaft Reborn Raspberry Pi Build Script" -ForegroundColor Blue
    Write-Host "================================================" -ForegroundColor Blue
}

function Write-Step {
    param([string]$Message)
    Write-Host "🔄 $Message" -ForegroundColor Yellow
}

function Write-Success {
    param([string]$Message)
    Write-Host "✅ $Message" -ForegroundColor Green
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host "❌ $Message" -ForegroundColor Red
}

Write-Header

# Detect container runtime
$containerCmd = if ($Podman) { "podman" } else { "docker" }

Write-Step "Checking for $containerCmd..."
if (-not (Get-Command $containerCmd -ErrorAction SilentlyContinue)) {
    if ($Podman) {
        Write-Error-Custom "Podman not found. Please install Podman or use Docker."
        exit 1
    } else {
        Write-Step "Docker not found, checking for Podman..."
        if (Get-Command podman -ErrorAction SilentlyContinue) {
            $containerCmd = "podman"
            Write-Success "Using Podman"
        } else {
            Write-Error-Custom "Neither Docker nor Podman found. Please install one."
            exit 1
        }
    }
} else {
    Write-Success "Using $containerCmd"
}

# Check for Docker Buildx (Docker only)
if ($containerCmd -eq "docker") {
    Write-Step "Checking for Docker Buildx..."
    try {
        & docker buildx version | Out-Null
        Write-Success "Docker Buildx available"
    } catch {
        Write-Error-Custom "Docker Buildx not found. Please install Docker Buildx."
        exit 1
    }
}

# Setup QEMU for ARM emulation
Write-Step "Setting up QEMU for ARM emulation..."
if ($containerCmd -eq "docker") {
    & docker run --rm --privileged multiarch/qemu-user-static --reset -p yes | Out-Null
} else {
    # Podman uses system QEMU
    if (-not (Get-Command qemu-arm-static -ErrorAction SilentlyContinue)) {
        Write-Error-Custom "QEMU not found. Please install qemu-user-static."
        exit 1
    }
}
Write-Success "QEMU setup complete"

function Build-Architecture {
    param(
        [string]$Arch,
        [string]$BuildCache
    )
    
    Write-Step "Building for $Arch ($BuildType)..."
    
    # Map architecture to platform
    $platform = "linux/$Arch"
    if ($Arch -eq "armhf") {
        $platform = "linux/arm/v7"
    }
    
    # Build arguments
    $buildArgs = @()
    if ($BuildCache) {
        $buildArgs += $BuildCache
    }
    
    # Build using Docker Buildx or Podman
    if ($containerCmd -eq "docker") {
        $buildArgs += @(
            "buildx", "build",
            "--platform", $platform,
            "--build-arg", "TARGET_ARCH=$Arch",
            "--build-arg", "BUILD_TYPE=$BuildType",
            "--tag", "crankshaft-reborn-${Arch}:${BuildType}",
            "--load",
            "-f", "Dockerfile.rpi",
            "."
        )
        
        & docker @buildArgs
        
        # Extract binaries
        $containerId = (& docker create "crankshaft-reborn-${Arch}:${BuildType}").Trim()
        & docker cp "${containerId}:/output/." "./build-output-${Arch}/"
        & docker rm $containerId | Out-Null
    } else {
        # Podman build
        $buildArgs += @(
            "build",
            "--arch", $Arch,
            "--build-arg", "TARGET_ARCH=$Arch",
            "--build-arg", "BUILD_TYPE=$BuildType",
            "--tag", "crankshaft-reborn-${Arch}:${BuildType}",
            "-f", "Dockerfile.rpi",
            "."
        )
        
        & podman @buildArgs
        
        # Extract binaries
        $containerId = (& podman create "crankshaft-reborn-${Arch}:${BuildType}").Trim()
        & podman cp "${containerId}:/output/." "./build-output-${Arch}/"
        & podman rm $containerId | Out-Null
    }
    
    Write-Success "Build completed for $Arch"
}

# Clean output directory if requested
if ($Clean) {
    Write-Step "Cleaning build output directories..."
    Remove-Item -Path "build-output-*" -Recurse -Force -ErrorAction SilentlyContinue
}

# Create output directories
New-Item -ItemType Directory -Path "build-output-armhf" -Force | Out-Null
New-Item -ItemType Directory -Path "build-output-arm64" -Force | Out-Null

# Prepare cache argument
$cacheArg = if ($NoCache) { "--no-cache" } else { "" }

# Build for specified architecture(s)
switch ($Architecture) {
    { $_ -in 'armhf', 'arm64' } {
        Build-Architecture -Arch $Architecture -BuildCache $cacheArg
    }
    'all' {
        Write-Step "Building for all architectures..."
        Build-Architecture -Arch 'armhf' -BuildCache $cacheArg
        Build-Architecture -Arch 'arm64' -BuildCache $cacheArg
        Write-Success "All architectures built successfully"
    }
}

Write-Step "Build results:"
if (Test-Path "build-output-armhf") {
    Write-Host ""
    Write-Host "ARMv7 (armhf) binaries:"
    Get-ChildItem -Path "build-output-armhf" -ErrorAction SilentlyContinue | Format-Table -AutoSize
}
if (Test-Path "build-output-arm64") {
    Write-Host ""
    Write-Host "ARM64 binaries:"
    Get-ChildItem -Path "build-output-arm64" -ErrorAction SilentlyContinue | Format-Table -AutoSize
}

Write-Host ""
Write-Success "Build complete!"
