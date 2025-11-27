# Media Player Extension Architecture

**Version:** 1.0  
**Last Updated:** November 27, 2025

## Overview

The Media Player extension provides audio and video playback capabilities to Crankshaft and other extensions. It uses a loosely-coupled architecture with GStreamer as the default media engine backend, allowing for future replacement without impacting consumers.

## Architecture Principles

- **Loose Coupling:** Media engine backend (GStreamer) is abstracted behind `IMediaEngine` interface
- **Swappable Backend:** Engine can be replaced (e.g., with VLC, FFmpeg, Qt Multimedia) without touching consumers
- **Extension-Friendly:** Other extensions can request playback via event bus
- **Multi-Source:** Supports local files, Bluetooth, radio/DAB, and streaming services
- **Capability-Based:** Uses Crankshaft's capability system for security

## High-Level Components

```
┌─────────────────────────────────────────────────────────────┐
│                     Media Player Extension                   │
├─────────────────────────────────────────────────────────────┤
│  MediaService (Public API)                                   │
│  ├── Play/Pause/Stop/Seek                                   │
│  ├── Queue Management                                        │
│  ├── Source Management                                       │
│  └── Event Publishing                                        │
├─────────────────────────────────────────────────────────────┤
│  IMediaEngine (Abstract Interface)                           │
│  ├── Playback Control                                        │
│  ├── State Management                                        │
│  ├── Metadata Extraction                                     │
│  └── Capability Discovery                                    │
├─────────────────────────────────────────────────────────────┤
│  GStreamerEngine (Concrete Implementation)                   │
│  ├── Pipeline Management                                     │
│  ├── Hardware Acceleration (RPi)                             │
│  ├── Audio/Video Output                                      │
│  └── Stream Handling                                         │
├─────────────────────────────────────────────────────────────┤
│  Source Adapters                                             │
│  ├── LocalFilesAdapter (TagLib + SQLite)                    │
│  ├── BluetoothAdapter (BlueZ + PipeWire)                    │
│  ├── RadioAdapter (RTL-SDR/DAB)                             │
│  └── StreamingAdapters (Mopidy/librespot)                   │
└─────────────────────────────────────────────────────────────┘
```

## Core Interfaces

### IMediaEngine Interface

Abstract interface for media playback engines. All engines must implement this contract.

```cpp
class IMediaEngine {
public:
    enum class State { Stopped, Playing, Paused, Buffering, Error };
    enum class MediaType { Audio, Video, Unknown };
    
    struct Capabilities {
        bool supportsVideo;
        bool supportsGapless;
        bool supportsHardwareDecode;
        bool supportsStreaming;
        bool supportsSeek;
        QStringList supportedFormats;
    };
    
    struct TrackMetadata {
        QString uri;
        QString title;
        QString artist;
        QString album;
        QString artworkUrl;
        qint64 durationMs;
        int bitrate;
        QString codec;
        MediaType type;
    };
    
    virtual ~IMediaEngine() = default;
    
    // Lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // Playback control
    virtual bool setUri(const QString& uri) = 0;
    virtual bool play() = 0;
    virtual bool pause() = 0;
    virtual bool stop() = 0;
    virtual bool seek(qint64 positionMs) = 0;
    
    // State queries
    virtual State state() const = 0;
    virtual qint64 position() const = 0;
    virtual qint64 duration() const = 0;
    
    // Volume & output
    virtual bool setVolume(int percent) = 0;  // 0-100
    virtual int volume() const = 0;
    virtual bool setMuted(bool muted) = 0;
    virtual bool isMuted() const = 0;
    
    // Metadata
    virtual TrackMetadata currentMetadata() const = 0;
    virtual bool extractMetadata(const QString& uri, TrackMetadata& out) = 0;
    
    // Capabilities
    virtual Capabilities capabilities() const = 0;
    
    // Callbacks (engine -> consumer)
    std::function<void(State)> onStateChanged;
    std::function<void(qint64)> onPositionChanged;
    std::function<void(qint64)> onDurationChanged;
    std::function<void(const TrackMetadata&)> onMetadataChanged;
    std::function<void(const QString&)> onError;
    std::function<void()> onEndOfStream;
};
```

### MediaService API

The `MediaService` wraps the media engine and exposes a high-level API via event bus.

**Event Topics (emitted by media_player):**
- `media_player.state_changed` - `{state: string, track: object}`
- `media_player.position_changed` - `{position: int, duration: int}`
- `media_player.metadata_changed` - `{metadata: object}`
- `media_player.queue_changed` - `{queue: array}`
- `media_player.source_changed` - `{sourceId: string, sourceName: string}`
- `media_player.error` - `{code: string, message: string}`

**Event Commands (consumed by media_player):**
- `media_player.play` - `{uri: string}` or `{trackId: string}`
- `media_player.pause` - `{}`
- `media_player.stop` - `{}`
- `media_player.next` - `{}`
- `media_player.previous` - `{}`
- `media_player.seek` - `{position: int}` (milliseconds)
- `media_player.set_volume` - `{volume: int}` (0-100)
- `media_player.set_muted` - `{muted: bool}`
- `media_player.enqueue` - `{uri: string}` or `{trackId: string}`
- `media_player.dequeue` - `{index: int}`
- `media_player.clear_queue` - `{}`
- `media_player.select_source` - `{sourceId: string}`

### Source Adapter Interface

Adapters provide media URIs to the media engine. They can run in-process or as separate processes.

```cpp
struct SourceInfo {
    QString id;              // e.g., "local_files"
    QString name;            // e.g., "Local Library"
    QString type;            // "local", "bluetooth", "radio", "streaming"
    QStringList capabilities; // "seek", "gapless", "metadata", "playlists"
    QString icon;            // MDI icon name
};

class ISourceAdapter {
public:
    virtual ~ISourceAdapter() = default;
    
    virtual SourceInfo info() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // List available items (tracks, stations, devices)
    virtual QVariantList listItems(const QString& category = "") = 0;
    
    // Resolve track ID to playable URI
    virtual QString resolveUri(const QString& trackId) = 0;
    
    // Search (optional)
    virtual QVariantList search(const QString& query) = 0;
    
    // Callbacks
    std::function<void(const QString&)> onAvailabilityChanged;
    std::function<void()> onLibraryUpdated;
};
```

## GStreamer Implementation

### GStreamerEngine

The default implementation uses GStreamer 1.x with the following pipeline strategy:

**Audio Pipeline:**
```
uridecodebin → audioconvert → audioresample → autoaudiosink
```

**Video Pipeline:**
```
uridecodebin → videoconvert → videoscale → autovideosink
```

**Features:**
- Hardware acceleration via `v4l2h264dec` on Raspberry Pi
- Gapless playback using `about-to-finish` signal
- Metadata extraction via `discoverer` API
- PipeWire/PulseAudio automatic sink selection
- Error recovery and reconnection

### Dependencies

Required GStreamer packages (Debian/Raspberry Pi OS):
```bash
libgstreamer1.0-dev
libgstreamer-plugins-base1.0-dev
libgstreamer-plugins-good1.0-dev
libgstreamer-plugins-bad1.0-dev
gstreamer1.0-plugins-ugly  # for MP3
gstreamer1.0-libav         # for AAC, etc.
gstreamer1.0-alsa
gstreamer1.0-pulseaudio
gstreamer1.0-pipewire      # preferred on modern systems
```

For hardware acceleration on Raspberry Pi:
```bash
gstreamer1.0-omx           # RPi 3
gstreamer1.0-v4l2          # RPi 4+
```

### CMake Integration

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_PBUTILS REQUIRED gstreamer-pbutils-1.0)

target_include_directories(MediaPlayerExtension PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${GSTREAMER_PBUTILS_INCLUDE_DIRS}
)

target_link_libraries(MediaPlayerExtension PRIVATE
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${GSTREAMER_PBUTILS_LIBRARIES}
)
```

## Source Adapters

### LocalFilesAdapter

**Technology:** TagLib (metadata), SQLite (library index)

**Features:**
- Recursive directory scanning with inotify watch
- Metadata extraction (ID3v2, Vorbis Comments, APEv2, MP4)
- Playlist management (M3U, PLS)
- Album artwork extraction and caching
- Search by title, artist, album, genre
- Smart playlists based on metadata

**Database Schema:**
```sql
CREATE TABLE tracks (
    id INTEGER PRIMARY KEY,
    uri TEXT UNIQUE NOT NULL,
    title TEXT,
    artist TEXT,
    album TEXT,
    album_artist TEXT,
    genre TEXT,
    year INTEGER,
    track_number INTEGER,
    duration_ms INTEGER,
    bitrate INTEGER,
    sample_rate INTEGER,
    file_size INTEGER,
    artwork_hash TEXT,
    last_modified INTEGER,
    last_played INTEGER,
    play_count INTEGER,
    date_added INTEGER
);

CREATE TABLE playlists (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    created INTEGER,
    modified INTEGER
);

CREATE TABLE playlist_tracks (
    playlist_id INTEGER,
    track_id INTEGER,
    position INTEGER,
    FOREIGN KEY(playlist_id) REFERENCES playlists(id),
    FOREIGN KEY(track_id) REFERENCES tracks(id)
);

CREATE INDEX idx_tracks_artist ON tracks(artist);
CREATE INDEX idx_tracks_album ON tracks(album);
CREATE INDEX idx_tracks_genre ON tracks(genre);
CREATE INDEX idx_tracks_year ON tracks(year);
```

### BluetoothAdapter

**Technology:** BlueZ (device management), PipeWire (audio routing)

**Features:**
- Device discovery and pairing via BlueZ D-Bus API
- A2DP audio streaming
- AVRCP metadata (track info, playback state)
- Multiple device support
- Automatic reconnection

**Flow:**
1. Extension uses BluetoothCapability to discover devices
2. User pairs device via system or UI
3. BluetoothAdapter detects A2DP connection
4. Adapter creates GStreamer URI: `pipewiresrc → ...`
5. AVRCP metadata forwarded to MediaService

### RadioAdapter

**Technology:** RTL-SDR (USB dongle), welle.io or SoapySDR (DAB decoding)

**Features:**
- FM/AM tuning via rtl_fm
- DAB+ decoding via welle.io
- Station presets and scanning
- RDS/RDBS metadata
- Signal strength monitoring

**Adapter Modes:**

**FM Mode:**
```
rtl_fm -f 95.5M -M fm -s 200k -r 48k - | gst-launch-1.0 fdsrc ! ...
```

**DAB Mode (welle.io):**
```cpp
// Use welle.io as library or spawn process
// Output PCM to named pipe or memory buffer
// Feed to GStreamer via appsrc
```

**Configuration:**
```json
{
  "radio": {
    "device_index": 0,
    "presets": [
      {"frequency": 95.5, "name": "BBC Radio 2", "type": "FM"},
      {"service_id": "0xC221", "name": "BBC Radio 1", "type": "DAB"}
    ]
  }
}
```

### StreamingAdapters (Mopidy-based)

**Technology:** Mopidy music server (Python), JSON-RPC API

**Supported Services:**
- Spotify (via mopidy-spotify)
- SoundCloud (via mopidy-soundcloud)
- TuneIn (via mopidy-tunein)
- YouTube (via mopidy-youtube)

**Architecture:**
- Mopidy runs as separate process (systemd service or spawned by adapter)
- Adapter communicates via HTTP JSON-RPC (localhost:6680)
- Mopidy outputs to PipeWire/PulseAudio
- GStreamer plays from Mopidy's audio output or via Mopidy's track URIs

**Example Mopidy Configuration (`~/.config/mopidy/mopidy.conf`):**
```ini
[spotify]
enabled = true
username = <user>
client_id = <client_id>
client_secret = <client_secret>

[audio]
output = pipewiresink

[http]
hostname = 127.0.0.1
port = 6680
```

**Adapter Flow:**
1. Adapter starts Mopidy process (if not already running)
2. Authenticates and queries available playlists
3. User selects track
4. Adapter gets track URI from Mopidy
5. Passes URI to GStreamer or routes Mopidy audio output

## Video Support

### Video Output

GStreamer supports video output for other extensions (e.g., reverse camera, video player).

**QML Video Surface:**
```qml
import QtMultimedia
import CrankshaftReborn.MediaPlayer 1.0

VideoOutput {
    id: videoOutput
    source: MediaPlayer.videoSurface
    fillMode: VideoOutput.PreserveAspectFit
}
```

**C++ Integration:**
```cpp
// GStreamerEngine exposes QAbstractVideoSurface
QAbstractVideoSurface* GStreamerEngine::videoSurface() const;

// Other extensions can request video surface via MediaService
```

### Hardware Acceleration

**Raspberry Pi 4:**
- Use `v4l2h264dec` for H.264
- Use `v4l2mpeg2dec` for MPEG-2
- GPU memory split: at least 128MB

**Pipeline Example:**
```
uridecodebin → v4l2h264dec → glupload → glcolorconvert → qmlglsink
```

## QML Integration

### Models

**NowPlayingModel:**
```qml
NowPlayingModel {
    id: nowPlaying
    onTrackChanged: {
        titleLabel.text = nowPlaying.title
        artistLabel.text = nowPlaying.artist
        artworkImage.source = nowPlaying.artworkUrl
    }
}
```

**QueueModel:**
```qml
ListView {
    model: QueueModel { id: queueModel }
    delegate: ItemDelegate {
        text: model.title + " - " + model.artist
        onClicked: MediaPlayer.playIndex(model.index)
    }
}
```

**SourcesModel:**
```qml
ComboBox {
    model: SourcesModel { id: sources }
    textRole: "name"
    onActivated: MediaPlayer.selectSource(sources.data(index, "id"))
}
```

### Playback Controls

```qml
import CrankshaftReborn.Events 1.0
import CrankshaftReborn.MediaPlayer 1.0

Row {
    Button {
        icon.name: "mdi:skip-previous"
        onClicked: EventBridge.publish("media_player.previous", {})
    }
    Button {
        icon.name: MediaPlayer.playing ? "mdi:pause" : "mdi:play"
        onClicked: MediaPlayer.playing ? 
            EventBridge.publish("media_player.pause", {}) :
            EventBridge.publish("media_player.play", {})
    }
    Button {
        icon.name: "mdi:skip-next"
        onClicked: EventBridge.publish("media_player.next", {})
    }
}

Slider {
    from: 0
    to: MediaPlayer.duration
    value: MediaPlayer.position
    onMoved: EventBridge.publish("media_player.seek", {position: value})
}
```

## Extension API for Video Consumers

Other extensions can request video playback:

**Example: Reverse Camera Extension**

```cpp
// Request video surface from MediaService
eventCap_->emitEvent("request_video_surface", {
    {"extensionId", "reverse_camera"},
    {"priority", "high"}
});

// MediaService responds with surface ID
eventCap_->subscribe("media_player.video_surface_granted", [](const QVariantMap& data) {
    int surfaceId = data["surfaceId"].toInt();
    // Use surface for rendering camera feed
});
```

## Testing Strategy

### Unit Tests

- Mock `IMediaEngine` for MediaService tests
- Mock `ISourceAdapter` for source management tests
- Isolated tests for queue, playlist, and state logic

### Integration Tests

- GStreamer pipeline tests with sample media files
- Adapter tests with mock data
- Event bus integration tests

### Hardware Tests

- Raspberry Pi 4 with RTL-SDR dongle
- Bluetooth A2DP device pairing
- Hardware video decode performance

### CI/CD

- GitHub Actions: unit tests on Ubuntu with GStreamer
- Optional: hardware test runner on Raspberry Pi (self-hosted)

## Milestones

### Milestone 1: Core Engine & Local Files (3-4 weeks)
- [x] Define IMediaEngine interface
- [ ] Implement GStreamerEngine
- [ ] Create MediaService with event bus API
- [ ] Implement LocalFilesAdapter with TagLib + SQLite
- [ ] Basic QML UI (NowPlaying, Queue, PlaybackControls)
- [ ] Unit tests and documentation

### Milestone 2: Bluetooth Support (2 weeks)
- [ ] Implement BluetoothAdapter
- [ ] A2DP streaming via PipeWire
- [ ] AVRCP metadata forwarding
- [ ] Bluetooth device selector UI

### Milestone 3: Radio/DAB (3 weeks)
- [ ] Implement RadioAdapter
- [ ] FM tuning via rtl_fm
- [ ] DAB decoding via welle.io
- [ ] Station presets and RDS metadata
- [ ] Radio tuner UI

### Milestone 4: Streaming Services (3-4 weeks)
- [ ] Mopidy integration adapter
- [ ] Spotify via mopidy-spotify
- [ ] OAuth flow for credentials
- [ ] Streaming service browser UI

### Milestone 5: Video & Polish (2 weeks)
- [ ] Video output surface API
- [ ] Hardware decode on Raspberry Pi
- [ ] QML VideoOutput integration
- [ ] Performance optimization
- [ ] End-user documentation

## Security Considerations

- **Adapter Sandboxing:** Run adapters as separate processes with limited filesystem/network access
- **Credential Storage:** Use system keyring for OAuth tokens
- **IPC Security:** Authenticate adapter connections with tokens
- **Input Validation:** Sanitise URIs and metadata from adapters
- **Resource Limits:** Enforce CPU/memory limits on adapter processes

## Performance Targets

- **Startup Time:** < 500ms to ready state
- **Track Change:** < 200ms gapless transition
- **Memory Usage:** < 50MB base + 20MB per adapter
- **CPU Usage (Idle):** < 2% on Raspberry Pi 4
- **CPU Usage (Playback):** < 15% for audio, < 30% for H.264 video with hardware decode

## Future Enhancements

- Multi-room audio (sync across multiple devices)
- Equaliser and audio effects (via GStreamer plugins)
- Lyrics display with time-sync
- Podcast support with episode management
- Car-specific features (steering wheel control, speed-dependent volume)
- Android Auto media integration (AASDK passthrough)

## References

- GStreamer Documentation: https://gstreamer.freedesktop.org/documentation/
- TagLib: https://taglib.org/
- Mopidy: https://mopidy.com/
- welle.io: https://www.welle.io/
- BlueZ: http://www.bluez.org/
- PipeWire: https://pipewire.org/

---

**Document Status:** Draft for review  
**Next Review:** After Milestone 1 completion
