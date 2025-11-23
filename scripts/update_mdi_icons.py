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
"""Utility script to refresh a curated subset of Material Design Icons.

This avoids bundling the full (large) icon set while enabling consistent icon
naming via the IconRegistry (mdi:<name>). The script fetches SVGs for the
specified names from the upstream repository.

Licensing: Material Design Icons are provided under Apache License 2.0.
This is compatible with GPLv3 distribution; retain notices when expanding set.

Usage (run under Python 3):
    python scripts/update_mdi_icons.py

Environment:
    Requires internet access and 'requests' library.

Notes:
    - The curated list is intentionally small to keep binary size low.
    - Add new icon names to ICONS list then re-run the script.
"""
from __future__ import annotations
import os
import sys
from typing import List

try:
    import requests
except ImportError:
    print("The 'requests' package is required. Install with: pip install requests", file=sys.stderr)
    sys.exit(1)

# Upstream raw base (GitHub repo snapshot)
RAW_BASE = "https://raw.githubusercontent.com/Templarian/MaterialDesign/master/svg"
# Curated icon list (without mdi- prefix, using simplified names aligned with registry)
ICONS: List[str] = [
    "home",
    "cog",
    "map",
    "music",
    "bluetooth",
    "play",
    "pause",
    "stop",
    "skip-next",
    "skip-previous",
]

TARGET_DIR = os.path.join("assets", "icons", "mdi")

# Mapping from upstream file name to local canonical name (remove hyphens for some, keep others as underscores)
RENAME_MAP = {
    "skip-next": "skip_next",
    "skip-previous": "skip_previous",
}

def fetch_icon(name: str) -> str:
    url = f"{RAW_BASE}/mdi-{name}.svg"
    resp = requests.get(url, timeout=15)
    if resp.status_code != 200:
        raise RuntimeError(f"Failed to fetch {name}: HTTP {resp.status_code}")
    return resp.text

def main() -> int:
    os.makedirs(TARGET_DIR, exist_ok=True)
    updated = 0
    for icon in ICONS:
        try:
            svg = fetch_icon(icon)
        except Exception as e:  # noqa: BLE001
            print(f"Warning: {icon} skipped: {e}")
            continue
        local_name = RENAME_MAP.get(icon, icon)
        # Normalise to underscore style for consistency with existing set
        local_name = local_name.replace('-', '_')
        path = os.path.join(TARGET_DIR, f"{local_name}.svg")
        with open(path, 'w', encoding='utf-8') as f:
            f.write(svg)
        updated += 1
        print(f"Updated {path}")
    print(f"Done. {updated} icons refreshed.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
