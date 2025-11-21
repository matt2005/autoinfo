# WSL2 Debian Development Environment Setup

This guide provides step-by-step instructions for setting up a complete development environment for Crankshaft Reborn in WSL2 Debian from scratch.

## Prerequisites

- Windows 10/11 with WSL2 enabled
- Fresh Debian WSL2 installation
- Internet connection

## Quick Setup (Automated)

For a quick automated setup, run the provided script:

```bash
# From Windows PowerShell, access your WSL2 instance
wsl

# Navigate to the project directory
cd /mnt/c/Users/<your-username>/path/to/crankshaft_reborn

# Make the setup script executable and run it
chmod +x scripts/wsl2-dev-setup.sh
./scripts/wsl2-dev-setup.sh
```

The script will:
1. Update system packages
2. Install all required development tools and Qt6 dependencies
3. Configure the build environment
4. Build the project
5. Set up VNC for headless display testing

## Manual Setup (Step-by-Step)

If you prefer to set up manually or need to troubleshoot, follow these detailed steps:

### 1. Update System Packages

```bash
# Update package lists and upgrade existing packages
sudo apt update
sudo apt upgrade -y
```

### 2. Install Build Essentials

```bash
# Install GCC, G++, Make, and other build tools
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    ca-certificates
```

### 3. Install Qt6 Development Packages

Crankshaft Reborn uses Qt6. Install all required Qt6 modules:

```bash
sudo apt install -y \
    qt6-base-dev \
    qt6-declarative-dev \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-window \
    libqt6websockets6-dev \
    libqt6network6 \
    qt6-positioning-dev \
    libqt6positioningquick6 \
    qml6-module-qtpositioning \
    qml6-module-qtlocation
```

**Note:** `qml6-module-qtlocation` is available in Debian Trixie (13) and later. It provides full map functionality for the navigation extension.

### 4. Install Additional Development Tools

```bash
# Install useful development utilities
sudo apt install -y \
    clang-format \
    clang-tidy \
    gdb \
    valgrind \
    strace \
    vim \
    nano
```

### 5. Verify Qt6 Installation

```bash
# Check Qt6 version
qmake6 --version

# Verify Qt6 modules location
ls -la /usr/lib/x86_64-linux-gnu/qt6/qml/

# Check for QtPositioning module
ls -la /usr/lib/x86_64-linux-gnu/qt6/qml/QtPositioning/
```

Expected output should show Qt 6.4.2 and the QtPositioning QML module.

### 6. Clone the Repository (if not already present)

```bash
# If accessing from Windows filesystem
cd /mnt/c/Users/<your-username>/path/to/repos

# Or clone fresh in WSL filesystem (faster)
cd ~
git clone https://github.com/matt2005/autoinfo.git crankshaft_reborn
cd crankshaft_reborn
```

**Performance Tip:** Building in WSL native filesystem (`/home/user/`) is significantly faster than Windows filesystem (`/mnt/c/`).

### 7. Configure the Build

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc \
    ..
```

**Build Types:**
- `Debug`: Full debugging symbols, no optimisation (for development)
- `Release`: Optimised, no debug symbols (for production)
- `RelWithDebInfo`: Optimised with debug symbols (for profiling)

### 8. Build the Project

```bash
# Build using all available cores
cmake --build . -j$(nproc)

# Or using Ninja directly
ninja
```

Build output should show:
- Core libraries building (CrankshaftCore, CrankshaftUI, CrankshaftExtensions)
- Extension modules (NavigationExtension)
- Main executable (CrankshaftReborn)

### 9. Verify Build Artifacts

```bash
# Check that executable exists
ls -lh ./CrankshaftReborn

# Check that extension shared libraries exist
ls -lh ./extensions/navigation/libNavigationExtension.so

# Verify assets were copied
ls -la ./assets/qml/
ls -la ./extensions/navigation/qml/
```

### 10. Install VNC for Headless Display

Since WSL2 doesn't have a display server by default, use Qt's VNC platform plugin:

```bash
# VNC dependencies are already included with Qt6
# Verify Qt platform plugins
ls /usr/lib/x86_64-linux-gnu/qt6/plugins/platforms/

# Should show libqvnc.so
```

## Running the Application

### Option 1: VNC Display (Headless)

```bash
# From build directory
cd build

# Run with VNC platform and specific resolution
QT_QPA_PLATFORM=vnc QT_QPA_VNC_SIZE=1024x600 ./CrankshaftReborn

# With detailed debugging output
QT_DEBUG_PLUGINS=1 QT_QPA_PLATFORM=vnc QT_QPA_VNC_SIZE=1024x600 ./CrankshaftReborn
```

**Connect to VNC:**
1. From Windows, install a VNC client (e.g., TightVNC Viewer, RealVNC)
2. Connect to `localhost:5900`
3. You should see the Crankshaft Reborn interface

**VNC Configuration:**
- Default port: `5900`
- Resolution: `1024x600` (configurable via `QT_QPA_VNC_SIZE`)
- Password: None by default (set with `QT_QPA_VNC_PASSWORD`)

### Option 2: X11 Forwarding (with X Server)

If you have an X server on Windows (VcXsrv, Xming):

```bash
# Set DISPLAY environment variable
export DISPLAY=:0

# Run normally
cd build
./CrankshaftReborn
```

**Setting up VcXsrv on Windows:**
1. Download and install VcXsrv from SourceForge
2. Launch XLaunch with settings: Multiple windows, Display 0, Start no client, Disable access control
3. In WSL, export DISPLAY

### Option 3: Testing Without Display

For automated testing or CI/CD:

```bash
# Run with timeout to prevent hanging
timeout 10 env QT_QPA_PLATFORM=vnc ./CrankshaftReborn

# Capture specific output
timeout 10 env QT_QPA_PLATFORM=vnc ./CrankshaftReborn 2>&1 | grep -i "extension\|loaded\|error"
```

## Development Workflow

### Building After Changes

```bash
# Quick rebuild (only changed files)
cd build
ninja

# Full rebuild (clean first)
ninja clean
ninja

# Or using CMake
cmake --build . --clean-first
```

### Running Tests

```bash
cd build
ctest --output-on-failure

# Run specific test
ctest -R extension_test -V
```

### Debugging with GDB

```bash
# Run under GDB
gdb ./CrankshaftReborn

# Inside GDB
(gdb) set environment QT_QPA_PLATFORM vnc
(gdb) set environment QT_QPA_VNC_SIZE 1024x600
(gdb) run
```

### Code Formatting

```bash
# Format all source files
find src extensions -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Or use the provided script
./format.sh
```

### Analysing Build Output

```bash
# Verbose build to see full compile commands
cmake --build . --verbose

# Check compilation database
cat compile_commands.json | jq .
```

## Common Issues and Solutions

### Issue: Qt6 modules not found

```
CMake Error: Could not find Qt6Core
```

**Solution:**
```bash
# Install Qt6 base packages
sudo apt install qt6-base-dev qt6-declarative-dev

# Set Qt6 path explicitly
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6
```

### Issue: QML module import errors

```
qml: module "QtQuick" is not installed
```

**Solution:**
```bash
# Install QML modules
sudo apt install qml6-module-qtquick qml6-module-qtquick-controls

# Verify installation
ls /usr/lib/x86_64-linux-gnu/qt6/qml/QtQuick/
```

### Issue: VNC not working

```
Could not find the Qt platform plugin "vnc"
```

**Solution:**
```bash
# Check Qt plugins
export QT_DEBUG_PLUGINS=1
./CrankshaftReborn

# Verify VNC plugin exists
ls /usr/lib/x86_64-linux-gnu/qt6/plugins/platforms/libqvnc.so
```

### Issue: Slow build performance

**Solution:**
- Move project to WSL native filesystem (`~/projects/` instead of `/mnt/c/`)
- Use Ninja instead of Make: `cmake -G Ninja`
- Use parallel builds: `ninja -j$(nproc)`

### Issue: Permission errors accessing Windows filesystem

**Solution:**
```bash
# Fix WSL file permissions
sudo umount /mnt/c
sudo mount -t drvfs C: /mnt/c -o metadata,uid=1000,gid=1000

# Or add to /etc/wsl.conf
sudo tee -a /etc/wsl.conf << EOF
[automount]
options = "metadata,uid=1000,gid=1000"
EOF
```

### Issue: Extension not loading

```
qml: Failed to load extension view
```

**Solution:**
```bash
# Verify extension assets were copied
ls -la build/extensions/navigation/

# Check manifest exists
cat build/extensions/navigation/manifest.json

# Verify QML resource paths
strings build/extensions/navigation/libNavigationExtension.so | grep -i "qrc\|navigation"
```

## Performance Optimisation

### Build Speed

```bash
# Use ccache for faster rebuilds
sudo apt install ccache
export PATH=/usr/lib/ccache:$PATH

# Or configure CMake to use ccache
cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..
```

### Runtime Performance

```bash
# Build with optimisations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Profile with perf (if available)
perf record ./CrankshaftReborn
perf report
```

## WSL Configuration Tips

### Increase WSL Memory Limit

Create or edit `%USERPROFILE%\.wslconfig`:

```ini
[wsl2]
memory=8GB
processors=4
swap=4GB
```

Restart WSL:
```powershell
wsl --shutdown
```

### Enable systemd (Debian 12+)

Edit `/etc/wsl.conf`:
```ini
[boot]
systemd=true
```

### File Watching for Auto-Rebuild

```bash
# Install inotify-tools
sudo apt install inotify-tools

# Watch for changes and rebuild
while inotifywait -r -e modify,create,delete src/ extensions/; do
    cmake --build build
done
```

## IDE Integration

### Visual Studio Code

1. Install "Remote - WSL" extension
2. Open project: `code .` from WSL terminal
3. Install C++ extension pack in WSL
4. Configure IntelliSense:

`.vscode/c_cpp_properties.json`:
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/include/x86_64-linux-gnu/qt6"
            ],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64",
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    "version": 4
}
```

### CLion

1. Settings → Build, Execution, Deployment → Toolchains
2. Add WSL toolchain
3. CMake will auto-detect Qt6

## Continuous Integration

Example GitHub Actions workflow for WSL build:

```yaml
name: WSL Build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Dependencies
        run: |
          sudo apt update
          sudo apt install -y qt6-base-dev qt6-declarative-dev
      - name: Build
        run: |
          mkdir build && cd build
          cmake -G Ninja ..
          ninja
      - name: Test
        run: |
          cd build
          timeout 10 env QT_QPA_PLATFORM=vnc ./CrankshaftReborn || true
```

## Next Steps

1. **Set up cross-compilation** for Raspberry Pi target (see `docs/cross_compilation.md`)
2. **Configure remote debugging** for Raspberry Pi deployment
3. **Set up extension development environment** (see `docs/extension_development.md`)
4. **Integrate with CI/CD** for automated testing

## Additional Resources

- [Qt6 Documentation](https://doc.qt.io/qt-6/)
- [CMake Documentation](https://cmake.org/documentation/)
- [WSL2 Documentation](https://docs.microsoft.com/windows/wsl/)
- [Project Extension Development Guide](./extension_development.md)
- [Architecture Documentation](./extension_ui_integration_options.md)

## Support

For issues specific to WSL2 setup:
1. Check system requirements match Debian 12 (Bookworm)
2. Ensure Qt6 version is 6.4.2 or compatible
3. Verify all dependencies are installed
4. Check build logs for specific errors
5. Open an issue on GitHub with full build output

---

**Last Updated:** November 2025  
**Tested Environment:** Debian 12 (Bookworm) in WSL2, Qt 6.4.2
