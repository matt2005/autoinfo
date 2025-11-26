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

# Installs development tools and Qt6 dev libraries on Debian/Ubuntu (WSL friendly)

SUDO=""
if command -v sudo >/dev/null 2>&1; then
  SUDO="sudo"
fi

${SUDO} apt-get update
${SUDO} apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  clang-format \
  clang-tidy \
  cppcheck \
  doxygen \
  python3-pip \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-websockets-dev \
  qt6-multimedia-dev \
  qt6-positioning-dev \
  qt6-connectivity-dev \
  qt6-tools-dev \
  qt6-tools-dev-tools

# Install cmakelint via apt or pip (fallback)
if ! ${SUDO} apt-get install -y cmakelint; then
  python3 -m pip install --user --upgrade pip cmakelint
fi

echo "Dev tools + Qt6 installed. Ensure ~/.local/bin is on PATH for pip-installed tools."