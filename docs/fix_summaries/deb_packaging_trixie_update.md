[]: # ```plaintext
[]: #  * Project: OpenAuto
[]: #  * This file is part of openauto project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  openauto is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  openauto is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
[]: # ```

# DEB packaging and Trixie update

- Summary: Added Debian packaging via CPack with dynamic dependency resolution and switched Raspberry Pi Docker base to Debian Trixie.
- Changes:
  - CMake: Enabled `CPACK_GENERATOR=DEB`, `CPACK_DEBIAN_PACKAGE_SHLIBDEPS=ON`, and set maintainer/homepage metadata.
  - build.sh: Added optional packaging (`cpack -G DEB`) when 4th arg is `ON`.
  - Dockerfile.rpi: Moved to `debian:trixie-slim`, installed `dpkg-dev`, triggered packaging, and export `.deb` to `/output`.
  - CI: Linux job installs `dpkg-dev`, runs `cpack` for Release, and uploads `.deb`; RPi artifacts include `.deb` from container.
- Result: DEB packages are produced for amd64 and armhf/arm64 with correct `Depends` computed by `dpkg-shlibdeps`.

## Notes
- The Qt6 dev package names remain the same on Trixie (`qt6-base-dev`, `qt6-declarative-dev`, etc.). If Debian changes package names later, adjust Dockerfile accordingly.
- Component splitting (core libs vs app) can be added later via CPack components.
