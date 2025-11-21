# Qt6 Location Geoservices Plugin Installation

## Issue

The navigation extension was displaying the error:

```text
"The geoservices provider is not supported."
```

This prevented the QtLocation Map component from loading OSM map tiles, despite having `libqt6location6` and `qml6-module-qtlocation` installed.

## Root Cause

The `qt6-location-plugins` package was not installed, which contains the actual geoservices provider plugins needed by QtLocation to fetch map tiles from various providers (OSM, HERE, Mapbox, etc.).

## Solution

Install the missing Qt6 location plugins package:

```bash
sudo apt-get install -y qt6-location-plugins
```

## Verification

After installation, the following plugins are available:

- `/usr/lib/x86_64-linux-gnu/qt6/plugins/geoservices/libqtgeoservices_osm.so` - OpenStreetMap provider
- `/usr/lib/x86_64-linux-gnu/qt6/plugins/geoservices/libqtgeoservices_itemsoverlay.so` - Items overlay provider

The application now successfully loads the OSM plugin and can display map tiles.

## Related Packages

- `libqt6location6` - Qt6 Location library (core functionality)
- `qml6-module-qtlocation` - QML bindings for QtLocation
- `qt6-location-plugins` - Geoservices provider plugins (required for map tiles)

## Date

2025-11-21
