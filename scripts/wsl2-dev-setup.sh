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

set -e  # Exit on error
set -u  # Exit on undefined variable

# Colours for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Colour

# Script metadata
SCRIPT_NAME="WSL2 Development Environment Setup"
SCRIPT_VERSION="1.0.0"

echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
echo -e "${BLUE}  ${SCRIPT_NAME}${NC}"
echo -e "${BLUE}  Version: ${SCRIPT_VERSION}${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
echo ""

# Function to print status messages
print_status() {
    echo -e "${BLUE}==>${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}!${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if package is installed
package_installed() {
    dpkg -l "$1" 2>/dev/null | grep -q "^ii"
}

# Check if running in WSL
if ! grep -qi microsoft /proc/version; then
    print_warning "This script is designed for WSL2. Detected environment might not be WSL."
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Check if running as root
if [[ $EUID -eq 0 ]]; then
    print_error "This script should not be run as root. Run as normal user with sudo access."
    exit 1
fi

# Check sudo access and cache credentials
print_status "Checking sudo access..."
if ! sudo -n true 2>/dev/null; then
    print_warning "Sudo credentials needed. You may be prompted for your password."
    sudo -v || {
        print_error "Failed to obtain sudo access. Please ensure your user has sudo privileges."
        exit 1
    }
fi

# Keep sudo alive throughout the script
while true; do sudo -n true; sleep 50; kill -0 "$$" || exit; done 2>/dev/null &

print_status "Starting development environment setup..."
echo ""

# ============================================================================
# Step 1: System Update
# ============================================================================

print_status "Step 1: Updating system packages..."
sudo apt update
sudo apt upgrade -y
print_success "System packages updated"
echo ""

# ============================================================================
# Step 2: Install Build Essentials
# ============================================================================

print_status "Step 2: Installing build essentials..."

BUILD_PACKAGES=(
    "build-essential"
    "cmake"
    "ninja-build"
    "git"
    "pkg-config"
    "ca-certificates"
)

for package in "${BUILD_PACKAGES[@]}"; do
    if package_installed "$package"; then
        print_success "$package already installed"
    else
        print_status "Installing $package..."
        sudo apt install -y "$package"
        print_success "$package installed"
    fi
done

echo ""

# ============================================================================
# Step 3: Install Qt6 Development Packages
# ============================================================================

print_status "Step 3: Installing Qt6 development packages..."

QT6_PACKAGES=(
    "qt6-base-dev"
    "qt6-declarative-dev"
    "qml6-module-qtquick"
    "qml6-module-qtquick-controls"
    "qml6-module-qtquick-layouts"
    "qml6-module-qtquick-window"
    "libqt6websockets6-dev"
    "libqt6network6"
    "qt6-positioning-dev"
    "libqt6positioningquick6"
    "qml6-module-qtpositioning"
    "qml6-module-qtlocation"
)

for package in "${QT6_PACKAGES[@]}"; do
    if package_installed "$package"; then
        print_success "$package already installed"
    else
        print_status "Installing $package..."
        sudo apt install -y "$package"
        print_success "$package installed"
    fi
done

print_success "Qt6 development packages installed"
print_status "Note: qml6-module-qtlocation provides full map functionality for navigation"

echo ""

# ============================================================================
# Step 4: Install Development Tools
# ============================================================================

print_status "Step 4: Installing additional development tools..."

DEV_TOOLS=(
    "clang-format"
    "clang-tidy"
    "gdb"
    "valgrind"
    "strace"
    "vim"
    "nano"
)

for tool in "${DEV_TOOLS[@]}"; do
    if package_installed "$tool"; then
        print_success "$tool already installed"
    else
        print_status "Installing $tool..."
        sudo apt install -y "$tool" || print_warning "Failed to install $tool (optional)"
    fi
done

echo ""

# ============================================================================
# Step 5: Verify Qt6 Installation
# ============================================================================

print_status "Step 5: Verifying Qt6 installation..."

if command_exists qmake6; then
    QT_VERSION=$(qmake6 --version | grep "Using Qt version" | cut -d' ' -f4)
    print_success "Qt6 installed: version $QT_VERSION"
else
    print_error "qmake6 not found. Qt6 installation may have failed."
    exit 1
fi

# Check Qt6 QML modules path
QT6_QML_PATH="/usr/lib/x86_64-linux-gnu/qt6/qml"
if [[ -d "$QT6_QML_PATH" ]]; then
    print_success "Qt6 QML modules directory found: $QT6_QML_PATH"
    
    # Check for QtPositioning
    if [[ -d "$QT6_QML_PATH/QtPositioning" ]]; then
        print_success "QtPositioning QML module found"
    else
        print_warning "QtPositioning QML module not found"
    fi
else
    print_warning "Qt6 QML modules directory not found at expected location"
fi

echo ""

# ============================================================================
# Step 6: Verify VNC Support
# ============================================================================

print_status "Step 6: Verifying VNC platform plugin..."

QT6_PLUGINS_PATH="/usr/lib/x86_64-linux-gnu/qt6/plugins/platforms"
if [[ -f "$QT6_PLUGINS_PATH/libqvnc.so" ]]; then
    print_success "Qt VNC platform plugin found"
else
    print_warning "Qt VNC platform plugin not found. VNC display may not work."
fi

echo ""

# ============================================================================
# Step 7: Configure Build Directory
# ============================================================================

print_status "Step 7: Configuring build directory..."

# Detect project root (script should be in scripts/ subdirectory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

print_status "Project root: $PROJECT_ROOT"

cd "$PROJECT_ROOT"

# Check if build directory exists
if [[ -d "build" ]]; then
    print_warning "Build directory already exists"
    read -p "Remove existing build directory? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf build
        print_success "Removed existing build directory"
    fi
fi

# Create build directory
mkdir -p build
cd build

print_status "Configuring CMake with Ninja generator..."

cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc \
    .. || {
        print_error "CMake configuration failed"
        exit 1
    }

print_success "CMake configuration complete"
echo ""

# ============================================================================
# Step 8: Build the Project
# ============================================================================

print_status "Step 8: Building the project..."

# Get number of CPU cores
NUM_CORES=$(nproc)
print_status "Building with $NUM_CORES parallel jobs..."

cmake --build . -j"$NUM_CORES" || {
    print_error "Build failed"
    exit 1
}

print_success "Build complete"
echo ""

# ============================================================================
# Step 9: Verify Build Artifacts
# ============================================================================

print_status "Step 9: Verifying build artifacts..."

# Check main executable
if [[ -f "./CrankshaftReborn" ]]; then
    print_success "Main executable built: CrankshaftReborn"
else
    print_error "Main executable not found"
    exit 1
fi

# Check extension libraries
if [[ -f "./extensions/navigation/libNavigationExtension.so" ]]; then
    print_success "NavigationExtension built"
else
    print_warning "NavigationExtension library not found"
fi

# Check assets
if [[ -d "./assets/qml" ]]; then
    QML_COUNT=$(find ./assets/qml -name "*.qml" | wc -l)
    print_success "Found $QML_COUNT QML files in assets"
else
    print_warning "Assets directory not found"
fi

# Check extension assets
if [[ -d "./extensions/navigation/qml" ]]; then
    EXT_QML_COUNT=$(find ./extensions/navigation/qml -name "*.qml" | wc -l)
    print_success "Found $EXT_QML_COUNT extension QML files"
else
    print_warning "Extension QML files not found"
fi

echo ""

# ============================================================================
# Step 10: Test Run with VNC
# ============================================================================

print_status "Step 10: Testing application with VNC display..."

print_status "Running quick test (5 seconds)..."
timeout 5 env QT_QPA_PLATFORM=vnc QT_QPA_VNC_SIZE=1024x600 ./CrankshaftReborn 2>&1 | tail -20 || true

echo ""

# ============================================================================
# Setup Complete
# ============================================================================

echo -e "${GREEN}═══════════════════════════════════════════════════${NC}"
echo -e "${GREEN}  Setup Complete!${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════${NC}"
echo ""

print_success "Development environment is ready"
echo ""

echo -e "${BLUE}Next Steps:${NC}"
echo ""
echo "1. Run the application with VNC:"
echo "   cd build"
echo "   QT_QPA_PLATFORM=vnc QT_QPA_VNC_SIZE=1024x600 ./CrankshaftReborn"
echo ""
echo "2. Connect VNC client from Windows:"
echo "   - Install TightVNC Viewer or RealVNC"
echo "   - Connect to: localhost:5900"
echo ""
echo "3. For development:"
echo "   - Edit source files in src/ or extensions/"
echo "   - Rebuild: cd build && ninja"
echo "   - Format code: ./format.sh"
echo ""
echo "4. Documentation:"
echo "   - WSL2 Setup Guide: docs/wsl2_dev_setup.md"
echo "   - Extension Development: docs/extension_development.md"
echo "   - Architecture: docs/extension_ui_integration_options.md"
echo ""

print_status "Environment Summary:"
echo "  • Qt Version: $(qmake6 --version | grep "Using Qt version" | cut -d' ' -f4)"
echo "  • Build Type: Debug"
echo "  • Generator: Ninja"
echo "  • Compiler: $(g++ --version | head -n1)"
echo "  • Project Root: $PROJECT_ROOT"
echo "  • Build Directory: $PROJECT_ROOT/build"
echo ""

print_success "Happy coding! 🚗💨"
