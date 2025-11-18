# Crankshaft Reborn

An extensible automotive infotainment system designed for modern vehicles, optimised for Raspberry Pi 4 running Raspberry Pi OS.

## Overview

Crankshaft Reborn is a modular, open-source infotainment platform that provides a robust foundation for creating custom in-vehicle experiences. Built with Qt6 and C++, it offers a flexible extension framework similar to Home Assistant, allowing developers to create and share custom functionality.

## Key Features

- **Extensible Architecture**: Plugin-based system supporting C/C++, Python, and Node.js extensions
- **Event-Driven Design**: Powerful event bus for inter-component communication
- **WebSocket API**: Real-time communication for remote control and monitoring
- **Multi-Display Support**: Run different interfaces on multiple displays simultaneously
- **Modern UI**: Clean, responsive interface with light and dark mode support
- **Cross-Platform**: Works on Linux with physical displays (EGLFS) and virtual displays (VNC)
- **Extension Store**: Discover and install community-created extensions
- **Base Extensions**: Includes media player, navigation, and Bluetooth support out of the box

## Architecture

### Core Components

- **Application Core**: Main application lifecycle and initialization
- **Event Bus**: Publish-subscribe event system for component communication
- **WebSocket Server**: Real-time API for external clients
- **Extension Manager**: Loads, manages, and validates extensions

### Extension Types

- **Service**: Background services (e.g., Bluetooth, GPS)
- **UI**: User interface components (e.g., media player UI)
- **Integration**: Third-party integrations (e.g., Spotify, Android Auto)
- **Platform**: Platform-specific features (e.g., hardware controls)

## Requirements

### Hardware

- Raspberry Pi 4 (2GB+ RAM recommended)
- Display with HDMI or DSI connection
- USB sound card or I2S DAC (recommended)
- GPS module (optional, for navigation)
- Bluetooth adapter (optional, for Bluetooth features)

### Software

- Raspberry Pi OS (Bookworm or later)
- Qt6 (6.2 or later)
- CMake (3.16 or later)
- GCC/Clang with C++17 support

## Building

### Docker Build (Recommended)

The easiest way to build Crankshaft Reborn is using Docker, which provides a consistent build environment:

#### Linux/macOS

```bash
# Build with default settings (Release mode)
./docker-build.sh

# Build with tests enabled
./docker-build.sh Release ON

# Build in Debug mode
./docker-build.sh Debug OFF

# Build for Raspberry Pi (cross-compilation with QEMU)
./docker-build-rpi.sh armhf release    # Raspberry Pi 3 (32-bit)
./docker-build-rpi.sh arm64 release    # Raspberry Pi 4/5 (64-bit)
./docker-build-rpi.sh all release      # Build for all architectures
```

#### Windows (PowerShell)

```powershell
# Build with default settings (Release mode)
.\docker-build.ps1

# Build with tests enabled
.\docker-build.ps1 -BuildType Release -BuildTests ON

# Build in Debug mode
.\docker-build.ps1 -BuildType Debug -BuildTests OFF

# Build for Raspberry Pi (cross-compilation with QEMU)
.\docker-build-rpi.ps1 armhf release    # Raspberry Pi 3 (32-bit)
.\docker-build-rpi.ps1 arm64 release    # Raspberry Pi 4/5 (64-bit)
.\docker-build-rpi.ps1 all release      # Build for all architectures
```

The Docker build will:
1. Build a Docker image with all required dependencies
2. Compile the project inside the container
3. Output binaries to the `build/` directory (or `build-output-{arch}/` for Raspberry Pi builds) on your host machine

#### Raspberry Pi Cross-Compilation

The Raspberry Pi build uses Docker with QEMU emulation to natively compile ARM binaries:

- **armhf**: ARMv7 32-bit for Raspberry Pi 3 and compatible devices
- **arm64**: ARM64 64-bit for Raspberry Pi 4, 5, and newer devices

Requirements:
- Docker with Buildx support, or Podman
- QEMU user-mode emulation (automatically set up by the build script)

The build script (`docker-build-rpi.sh` / `docker-build-rpi.ps1`) will:
1. Set up QEMU for ARM emulation
2. Build using native ARM compilation in an emulated environment
3. Extract binaries to `build-output-{arch}/` directory

### Native Linux Build

If you prefer to build directly on your system:

```bash
# Install dependencies (Debian/Ubuntu/Raspberry Pi OS)
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-websockets-dev \
    libqt6websockets6-dev \
    qt6-multimedia-dev \
    qt6-positioning-dev \
    qt6-connectivity-dev

# Build the project
cd crankshaft_reborn
chmod +x scripts/build.sh
./scripts/build.sh Release
```

### Windows

```powershell
# Ensure Qt6 is installed
# Build the project
cd crankshaft_reborn
.\scripts\build.ps1 -BuildType Release
```

## Installation

### Install Runtime Dependencies

```bash
# Install required Qt6 QML runtime modules
sudo apt-get update
sudo apt-get install -y \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-window \
    qml6-module-qtqml-workerscript \
    libqt6qml6 \
    libqt6quick6 \
    qt6-qpa-plugins
```

### Install Application

```bash
# Install system-wide
cd build
sudo cmake --install .

# Run
CrankshaftReborn

# Run with VNC (for testing without physical display)
CrankshaftReborn -platform vnc:size=1024x600,port=5900

# Run with debug logging (verbose Qt plugin and QML output)
QT_DEBUG_PLUGINS=1 QT_LOGGING_RULES="*=true" CrankshaftReborn -platform vnc:size=1024x600,port=5900
```

## Configuration

Configuration files are located in `/etc/CrankshaftReborn/` (system-wide) or `~/.config/CrankshaftReborn/` (user-specific).

See `config/crankshaft.json` or `config/crankshaft.conf` for configuration options.

## Developing Extensions

Extensions allow you to add custom functionality to Crankshaft Reborn. See the [Extension Development Guide](docs/extension_development.md) for detailed instructions.

### Quick Start

1. Create a directory in `extensions/`
2. Add a `manifest.json` file
3. Implement your extension
4. Build and install

## Base Extensions

### Media Player

Full-featured audio and video player with support for:
- Multiple audio formats (MP3, FLAC, WAV, OGG, AAC)
- Video playback (MP4, MKV, AVI)
- Playlist management
- Equaliser
- Bluetooth audio streaming
- Radio

### Navigation

GPS navigation system featuring:
- Turn-by-turn navigation
- Real-time traffic updates
- Points of interest
- Offline maps support
- Voice guidance
- Multiple map providers (OpenStreetMap, Mapbox)

### Bluetooth Manager

Bluetooth connectivity for:
- Hands-free calling
- Audio streaming (A2DP)
- Contact synchronisation
- Multiple device support
- Auto-connect

## API Documentation

See [API Documentation](docs/api.md) for details on the core APIs and WebSocket protocol.

## Contributing

We welcome contributions! Please read our [Contributing Guide](docs/CONTRIBUTING.md) for details.

## Testing

```bash
# Build with tests
cmake .. -DBUILD_TESTS=ON

# Run tests
ctest
```

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Qt Project for the excellent Qt framework
- OpenCarDev community for inspiration and support
- OpenStreetMap for mapping data

## Support

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/opencardev/crankshaft_reborn/issues)
- **Discussions**: [GitHub Discussions](https://github.com/opencardev/crankshaft_reborn/discussions)

## Roadmap

- [ ] Complete base extension implementations
- [ ] Extension store implementation
- [ ] Android Auto integration
- [ ] Apple CarPlay support
- [ ] Voice control
- [ ] Advanced theming system
- [ ] Performance optimisations for Raspberry Pi
- [ ] Multi-language support
- [ ] OBD-II integration

## Authors

OpenCarDev Team

## Project Status

🚧 **Active Development** - This project is in active development. APIs may change before the 1.0.0 release.
