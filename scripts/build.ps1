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

param(
    [string]$BuildType = "Release"
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

Write-Host "Building Crankshaft Reborn..." -ForegroundColor Green
Write-Host "Build type: $BuildType"

# Create build directory
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Set-Location $BuildDir

# Configure
cmake .. `
    -DCMAKE_BUILD_TYPE="$BuildType" `
    -DBUILD_TESTS=ON `
    -DBUILD_EXTENSIONS=ON

# Build
cmake --build . --config $BuildType

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Binary location: $BuildDir\$BuildType\CrankshaftReborn.exe"
