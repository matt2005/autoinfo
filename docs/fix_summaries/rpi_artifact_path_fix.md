[]: # ```plaintext
[]: #  * Project: Crankshaft
[]: #  * This file is part of Crankshaft project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  Crankshaft is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  Crankshaft is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
[]: # ```plaintext

# Raspberry Pi artifact path fix

- Summary: Align Docker build output with CI artifact expectations.
- Context: RPi jobs succeeded but artifact upload warned: no files found at `build-output/CrankshaftReborn`, `build-output/libCrankshaftCore.a`, etc. The container `/output` was empty.
- Root cause: `Dockerfile.rpi` copied from `build-release/` or `build-debug/`, but `build.sh` writes to `build/` by default. Also case mismatch (`Debug` vs `debug`) prevented the debug branch from triggering.
- Fix: Copy artifacts from `build/` unconditionally after running `build.sh`.
- Change:
  - `Dockerfile.rpi`: replace copy section with:
    - `cp -r build/CrankshaftReborn /output/ || true`
    - `cp -r build/*.a /output/ || true`
    - `cp -r build/*.so* /output/ || true`
- Outcome: `/output` now contains expected binaries and the CI `build-raspberry-pi` jobs can upload artifacts from `build-output/`.

## Notes
- Consider parameterising `build.sh` to output to `build-release`/`build-debug` later for clearer separation, but current fix keeps changes minimal and consistent with existing Linux job.
