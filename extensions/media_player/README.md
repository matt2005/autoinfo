# Media Player Extension

Full-featured media player extension for Crankshaft providing audio and video playback capabilities.

## Features

- **GStreamer Backend:** High-performance media engine with hardware acceleration support
- **Multiple Sources:** Local files, Bluetooth, Radio/DAB, streaming services
- **Loose Coupling:** Swappable media engine via `IMediaEngine` interface
- **Event-Based API:** Control playback via event bus from any extension
- **Queue Management:** Playlist and queue support
- **Metadata Extraction:** Automatic tag reading and display
- **Video Support:** Hardware-accelerated video on Raspberry Pi

## Architecture

```text
MediaPlayerExtension
├── IMediaEngine (abstract interface)
├── GStreamerEngine (default implementation)
└── Source Adapters (future)
    ├── LocalFilesAdapter
    ├── BluetoothAdapter
    ├── RadioAdapter
    └── StreamingAdapters
```

## Installation

### System Dependencies

**Debian/Raspberry Pi OS:**

```bash
sudo apt-get install \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-good1.0-dev \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-alsa \
    gstreamer1.0-pulseaudio \
    gstreamer1.0-pipewire \
    gstreamer1.0-v4l2
```

### Building

The extension is built as part of the main Crankshaft project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

### Event API

The media player responds to events on the `media_player.*` topic:

**Play Media:**

```javascript
EventBridge.publish("media_player.play", {
    uri: "file:///media/music/song.mp3"
})
```

**Playback Control:**

```javascript
EventBridge.publish("media_player.pause", {})
EventBridge.publish("media_player.stop", {})
EventBridge.publish("media_player.next", {})
EventBridge.publish("media_player.seek", { position: 30000 }) // 30 seconds
```

**Volume Control:**

```javascript
EventBridge.publish("media_player.set_volume", { volume: 75 })
EventBridge.publish("media_player.set_muted", { muted: true })
```

**Queue Management:**

```javascript
EventBridge.publish("media_player.enqueue", { uri: "file:///path/to/track.mp3" })
EventBridge.publish("media_player.dequeue", { index: 2 })
EventBridge.publish("media_player.clear_queue", {})
```

### Events Published

Subscribe to these events to receive updates:

- `media_player.state_changed` - Playback state changes
- `media_player.position_changed` - Position updates (every 250ms)
- `media_player.duration_changed` - When duration is determined
- `media_player.metadata_changed` - Track metadata updates
- `media_player.queue_changed` - Queue modifications
- `media_player.error` - Playback errors
- `media_player.buffering_changed` - Stream buffering progress

## Configuration

Configuration items are available in the UI under **Media Player Settings**:

### Playback Settings

- Default volume (0-100%)
- Auto-play on connect
- Repeat mode (Off/One/All)
- Shuffle

### Audio Quality

- Equaliser enable/presets
- Volume normalisation

### Library Settings

- Library directories to scan
- Auto-scan on startup

## Supported Formats

**Audio:** mp3, flac, wav, ogg, opus, aac, m4a, wma  
**Video:** mp4, mkv, avi, webm, mov, flv

## Hardware Acceleration

On Raspberry Pi 4, the extension automatically uses V4L2 hardware decoders for:

- H.264 (v4l2h264dec)
- MPEG-2 (v4l2mpeg2dec)

Ensure GPU memory is set to at least 128MB in `/boot/config.txt`:

```ini
gpu_mem=128
```

## Extending the Media Player

### Creating a Custom Engine

To replace GStreamer with another backend:

1. Implement the `IMediaEngine` interface
2. Update `MediaPlayerExtension::initialize()` to instantiate your engine
3. Rebuild the extension

See `docs/media_player_extension.md` for detailed interface documentation.

### Creating Source Adapters

Source adapters provide media URIs to the engine:

1. Implement the `ISourceAdapter` interface (coming soon)
2. Register adapter with `MediaPlayerExtension`
3. Publish URIs via adapter callbacks

## Development Status

### Completed ✅

- IMediaEngine interface design
- GStreamerEngine implementation
- Event-based API
- Queue management
- Metadata extraction
- Hardware acceleration detection

### In Progress 🚧

- Source adapter framework
- QML UI components

### Planned 📋

- LocalFilesAdapter (TagLib + SQLite)
- BluetoothAdapter (BlueZ + PipeWire)
- RadioAdapter (RTL-SDR/DAB)
- Streaming service adapters (Spotify, Amazon Music)
- Video output API for other extensions
- Playlist management
- Equaliser support

## Testing

Unit tests can be run via:

```bash
cd build
ctest --test-dir . --output-on-failure
```

## Contributing

Contributions are welcome! See `CONTRIBUTING.md` in the project root.

## Licence

Copyright (C) 2025 OpenCarDev Team

This extension is part of Crankshaft and is licenced under the GNU General Public Licence v3.0 or later.

## Support

- **Documentation:** `docs/media_player_extension.md`
- **Issues:** <https://github.com/opencardev/crankshaft_reborn/issues>
- **Community:** <https://discord.gg/opencardev>
