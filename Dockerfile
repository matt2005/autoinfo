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

# Multi-architecture Debian build
# Supports: amd64, armhf (arm/v7), arm64 (arm64/v8)
# Debian versions: bookworm, trixie

ARG DEBIAN_VERSION=trixie
FROM debian:${DEBIAN_VERSION}-slim

# Build arguments
ARG BUILD_TYPE=Release
ARG DEBIAN_FRONTEND=noninteractive
ARG TARGETARCH

# Set locale to avoid encoding issues
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install build dependencies and Qt6
RUN apt-get update && apt-get install -y \
    # Core build tools
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    dpkg-dev \
    file \
    # Qt6 development packages
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-websockets-dev \
    qt6-multimedia-dev \
    qt6-positioning-dev \
    qt6-connectivity-dev \
    # Bluetooth support\
    bluez \
    bluez-tools \
    libbluetooth-dev \
    # Qt6 QML runtime modules (required for QML applications)
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-window \
    qml6-module-qtqml-workerscript \
    qml6-module-qtlocation \
    libqt6qml6 \
    libqt6quick6 \
    # Qt6 platform plugins
    qt6-qpa-plugins \
    qt6-location-plugins \
    # XKB (required for Qt Quick)
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    # Additional libraries
    libboost-system-dev \
    libboost-log-dev \
    qttools6-dev-tools \
    # GStreamer for media player
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-alsa \
    gstreamer1.0-pulseaudio \
    gstreamer1.0-tools \
    # Formatting tools
    clang-format \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /src

# Copy source code
COPY . .

# Create output directory
RUN mkdir -p /output

# Make build script executable
RUN chmod +x ./scripts/build.sh

# Build the project
RUN ./scripts/build.sh ${BUILD_TYPE} OFF build ON && \
    # Copy binaries to output
    (cp -r build/CrankshaftReborn /output/ || true) && \
    (cp -r build/*.a /output/ || true) && \
    (cp -r build/*.so* /output/ || true) && \
    (cp -r build/*.deb /output/ || true)

# Default command
CMD ["bash", "-c", "echo 'Crankshaft Reborn build container ready. Binaries are in /output/'"]
