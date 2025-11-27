/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */

#include "media_player_extension.hpp"
#include "GStreamerEngine.hpp"
#include <QDebug>
#include "../../src/core/config/ConfigManager.hpp"
#include "../../src/core/config/ConfigTypes.hpp"

namespace opencardev::crankshaft {
namespace extensions {
namespace media {

bool MediaPlayerExtension::initialize() {
    qInfo() << "Initialising Media Player extension...";

    eventCap_ = getCapability<core::capabilities::EventCapability>();
    if (!eventCap_) {
        qWarning() << "Event capability not granted; extension will be disabled.";
        return false;
    }

    // Create and initialise media engine (GStreamer by default)
    mediaEngine_ = std::make_unique<GStreamerEngine>();
    if (!mediaEngine_->initialize()) {
        qCritical() << "Failed to initialise media engine";
        return false;
    }

    qInfo() << "Media engine capabilities:";
    auto caps = mediaEngine_->capabilities();
    qInfo() << "  Video support:" << caps.supportsVideo;
    qInfo() << "  Gapless playback:" << caps.supportsGapless;
    qInfo() << "  Hardware decode:" << caps.supportsHardwareDecode;
    qInfo() << "  Streaming:" << caps.supportsStreaming;
    qInfo() << "  Seek:" << caps.supportsSeek;

    setupEngineCallbacks();
    setupEventHandlers();
    
    return true;
}

void MediaPlayerExtension::start() {
    qInfo() << "Starting Media Player extension...";
    
    // Publish initial state
    publishStateChanged();
}

void MediaPlayerExtension::stop() {
    qInfo() << "Stopping Media Player extension...";
    
    if (mediaEngine_) {
        mediaEngine_->stop();
    }
}

void MediaPlayerExtension::cleanup() {
    qInfo() << "Cleaning up Media Player extension...";
    
    if (mediaEngine_) {
        mediaEngine_->shutdown();
        mediaEngine_.reset();
    }
    
    eventCap_.reset();
}

void MediaPlayerExtension::registerConfigItems(core::config::ConfigManager* manager) {
    using namespace core::config;

    ConfigPage page;
    page.domain = "media";
    page.extension = "player";
    page.title = "Media Player Settings";
    page.description = "Configure media playback and library options";
    page.icon = "qrc:/icons/media.svg";

    // Playback Settings Section
    ConfigSection playbackSection;
    playbackSection.key = "playback";
    playbackSection.title = "Playback Settings";
    playbackSection.description = "Control media playback behavior";
    playbackSection.complexity = ConfigComplexity::Basic;

    ConfigItem volumeItem;
    volumeItem.key = "default_volume";
    volumeItem.label = "Default volume";
    volumeItem.description = "Default volume level when starting playback";
    volumeItem.type = ConfigItemType::Integer;
    volumeItem.defaultValue = 75;
    volumeItem.properties["minValue"] = 0;
    volumeItem.properties["maxValue"] = 100;
    volumeItem.unit = "%";
    volumeItem.complexity = ConfigComplexity::Basic;
    volumeItem.required = false;
    volumeItem.readOnly = false;
    playbackSection.items.append(volumeItem);

    ConfigItem autoPlayItem;
    autoPlayItem.key = "auto_play";
    autoPlayItem.label = "Auto-play on connect";
    autoPlayItem.description = "Automatically start playback when audio source connects";
    autoPlayItem.type = ConfigItemType::Boolean;
    autoPlayItem.defaultValue = true;
    autoPlayItem.complexity = ConfigComplexity::Basic;
    autoPlayItem.required = false;
    autoPlayItem.readOnly = false;
    playbackSection.items.append(autoPlayItem);

    ConfigItem repeatModeItem;
    repeatModeItem.key = "repeat_mode";
    repeatModeItem.label = "Repeat mode";
    repeatModeItem.description = "Default repeat mode for playlists";
    repeatModeItem.type = ConfigItemType::Selection;
    repeatModeItem.properties["options"] = QStringList{"Off", "One", "All"};
    repeatModeItem.defaultValue = "All";
    repeatModeItem.complexity = ConfigComplexity::Basic;
    repeatModeItem.required = false;
    repeatModeItem.readOnly = false;
    playbackSection.items.append(repeatModeItem);

    ConfigItem shuffleItem;
    shuffleItem.key = "shuffle";
    shuffleItem.label = "Enable shuffle";
    shuffleItem.description = "Shuffle playback order by default";
    shuffleItem.type = ConfigItemType::Boolean;
    shuffleItem.defaultValue = false;
    shuffleItem.complexity = ConfigComplexity::Basic;
    shuffleItem.required = false;
    shuffleItem.readOnly = false;
    playbackSection.items.append(shuffleItem);

    page.sections.append(playbackSection);

    // Audio Quality Section
    ConfigSection qualitySection;
    qualitySection.key = "quality";
    qualitySection.title = "Audio Quality";
    qualitySection.description = "Configure audio quality and processing";
    qualitySection.complexity = ConfigComplexity::Advanced;

    ConfigItem equalizerItem;
    equalizerItem.key = "equalizer";
    equalizerItem.label = "Enable equalizer";
    equalizerItem.description = "Enable audio equalizer for sound customization";
    equalizerItem.type = ConfigItemType::Boolean;
    equalizerItem.defaultValue = false;
    equalizerItem.complexity = ConfigComplexity::Advanced;
    equalizerItem.required = false;
    equalizerItem.readOnly = false;
    qualitySection.items.append(equalizerItem);

    ConfigItem equalizerPresetItem;
    equalizerPresetItem.key = "equalizer_preset";
    equalizerPresetItem.label = "Equalizer preset";
    equalizerPresetItem.description = "Audio equalizer preset";
    equalizerPresetItem.type = ConfigItemType::Selection;
    equalizerPresetItem.properties["options"] = QStringList{
        "Flat", "Pop", "Rock", "Jazz", "Classical", "Bass Boost", "Treble Boost", "Custom"};
    equalizerPresetItem.defaultValue = "Flat";
    equalizerPresetItem.complexity = ConfigComplexity::Advanced;
    equalizerPresetItem.required = false;
    equalizerPresetItem.readOnly = false;
    qualitySection.items.append(equalizerPresetItem);

    ConfigItem normalizationItem;
    normalizationItem.key = "volume_normalization";
    normalizationItem.label = "Volume normalization";
    normalizationItem.description = "Normalize volume levels across different tracks";
    normalizationItem.type = ConfigItemType::Boolean;
    normalizationItem.defaultValue = true;
    normalizationItem.complexity = ConfigComplexity::Advanced;
    normalizationItem.required = false;
    normalizationItem.readOnly = false;
    qualitySection.items.append(normalizationItem);

    page.sections.append(qualitySection);

    // Library Settings Section
    ConfigSection librarySection;
    librarySection.key = "library";
    librarySection.title = "Library Settings";
    librarySection.description = "Configure media library and scanning";
    librarySection.complexity = ConfigComplexity::Basic;

    ConfigItem libraryPathsItem;
    libraryPathsItem.key = "library_paths";
    libraryPathsItem.label = "Library directories";
    libraryPathsItem.description = "Directories to scan for media files";
    libraryPathsItem.type = ConfigItemType::MultiSelection;
    libraryPathsItem.properties["options"] =
        QStringList{"/media/music", "/media/usb", "/media/sdcard"};
    libraryPathsItem.defaultValue = QStringList{"/media/music"};
    libraryPathsItem.complexity = ConfigComplexity::Basic;
    libraryPathsItem.required = false;
    libraryPathsItem.readOnly = false;
    librarySection.items.append(libraryPathsItem);

    ConfigItem autoScanItem;
    autoScanItem.key = "auto_scan";
    autoScanItem.label = "Auto-scan library";
    autoScanItem.description = "Automatically scan for new media files on startup";
    autoScanItem.type = ConfigItemType::Boolean;
    autoScanItem.defaultValue = true;
    autoScanItem.complexity = ConfigComplexity::Basic;
    autoScanItem.required = false;
    autoScanItem.readOnly = false;
    librarySection.items.append(autoScanItem);

    page.sections.append(librarySection);

    // Register the page
    manager->registerConfigPage(page);
    qInfo() << "Media Player extension registered config items";
}

void MediaPlayerExtension::setupEngineCallbacks() {
    if (!mediaEngine_) {
        return;
    }

    // Connect engine signals to event publishing
    QObject::connect(mediaEngine_.get(), &IMediaEngine::stateChanged, 
            [this](IMediaEngine::State state) {
        Q_UNUSED(state);
        publishStateChanged();
    });

    QObject::connect(mediaEngine_.get(), &IMediaEngine::positionChanged,
            [this](qint64 position) {
        publishPositionChanged(position);
    });

    QObject::connect(mediaEngine_.get(), &IMediaEngine::durationChanged,
            [this](qint64 duration) {
        QVariantMap data;
        data["duration"] = duration;
        eventCap_->emitEvent("duration_changed", data);
    });

    QObject::connect(mediaEngine_.get(), &IMediaEngine::metadataChanged,
            [this](const IMediaEngine::TrackMetadata& metadata) {
        Q_UNUSED(metadata);
        publishMetadataChanged();
    });

    QObject::connect(mediaEngine_.get(), &IMediaEngine::error,
            [this](const QString& message) {
        publishError(message);
    });

    QObject::connect(mediaEngine_.get(), &IMediaEngine::endOfStream,
            [this]() {
        playNext();
    });

    QObject::connect(mediaEngine_.get(), &IMediaEngine::bufferingChanged,
            [this](int percent) {
        QVariantMap data;
        data["percent"] = percent;
        eventCap_->emitEvent("buffering_changed", data);
    });
}

void MediaPlayerExtension::setupEventHandlers() {
    if (!eventCap_) {
        qWarning() << "Event capability not available";
        return;
    }

    // Subscribe to playback control commands
    eventCap_->subscribe("media_player.play",
                         [this](const QVariantMap& data) { handlePlayCommand(data); });
    eventCap_->subscribe("media_player.pause",
                         [this](const QVariantMap& data) { handlePauseCommand(data); });
    eventCap_->subscribe("media_player.stop",
                         [this](const QVariantMap& data) { handleStopCommand(data); });
    eventCap_->subscribe("media_player.next",
                         [this](const QVariantMap& data) { handleNextCommand(data); });
    eventCap_->subscribe("media_player.previous",
                         [this](const QVariantMap& data) { handlePreviousCommand(data); });
    eventCap_->subscribe("media_player.seek",
                         [this](const QVariantMap& data) { handleSeekCommand(data); });
    eventCap_->subscribe("media_player.set_volume",
                         [this](const QVariantMap& data) { handleSetVolumeCommand(data); });
    eventCap_->subscribe("media_player.set_muted",
                         [this](const QVariantMap& data) { handleSetMutedCommand(data); });

    // Queue management
    eventCap_->subscribe("media_player.enqueue",
                         [this](const QVariantMap& data) { handleEnqueueCommand(data); });
    eventCap_->subscribe("media_player.dequeue",
                         [this](const QVariantMap& data) { handleDequeueCommand(data); });
    eventCap_->subscribe("media_player.clear_queue",
                         [this](const QVariantMap& data) { handleClearQueueCommand(data); });
}

void MediaPlayerExtension::handlePlayCommand(const QVariantMap& data) {
    if (!mediaEngine_) {
        return;
    }

    if (data.contains("uri")) {
        QString uri = data["uri"].toString();
        qInfo() << "Play command with URI:" << uri;
        
        currentTrackUri_ = uri;
        mediaEngine_->setUri(uri);
        mediaEngine_->play();
    } else {
        // Resume playback
        qInfo() << "Resume playback";
        mediaEngine_->play();
    }
}

void MediaPlayerExtension::handlePauseCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    
    if (!mediaEngine_) {
        return;
    }

    qInfo() << "Pause command";
    mediaEngine_->pause();
}

void MediaPlayerExtension::handleStopCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    
    if (!mediaEngine_) {
        return;
    }

    qInfo() << "Stop command";
    mediaEngine_->stop();
    currentTrackUri_.clear();
}

void MediaPlayerExtension::handleNextCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    
    qInfo() << "Next track command";
    playNext();
}

void MediaPlayerExtension::handlePreviousCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    
    qInfo() << "Previous track command";
    // TODO: Implement previous track logic with history
}

void MediaPlayerExtension::handleSeekCommand(const QVariantMap& data) {
    if (!mediaEngine_ || !data.contains("position")) {
        return;
    }

    qint64 position = data["position"].toLongLong();
    qInfo() << "Seek to:" << position << "ms";
    mediaEngine_->seek(position);
}

void MediaPlayerExtension::handleSetVolumeCommand(const QVariantMap& data) {
    if (!mediaEngine_ || !data.contains("volume")) {
        return;
    }

    int volume = data["volume"].toInt();
    qInfo() << "Set volume:" << volume;
    mediaEngine_->setVolume(volume);
}

void MediaPlayerExtension::handleSetMutedCommand(const QVariantMap& data) {
    if (!mediaEngine_ || !data.contains("muted")) {
        return;
    }

    bool muted = data["muted"].toBool();
    qInfo() << "Set muted:" << muted;
    mediaEngine_->setMuted(muted);
}

void MediaPlayerExtension::handleEnqueueCommand(const QVariantMap& data) {
    if (!data.contains("uri")) {
        return;
    }

    QString uri = data["uri"].toString();
    qInfo() << "Enqueue:" << uri;
    
    playbackQueue_.enqueue(uri);
    isQueueMode_ = true;
    publishQueueChanged();

    // If nothing is playing, start playback
    if (mediaEngine_ && mediaEngine_->state() == IMediaEngine::State::Stopped) {
        playFromQueue();
    }
}

void MediaPlayerExtension::handleDequeueCommand(const QVariantMap& data) {
    if (!data.contains("index")) {
        return;
    }

    int index = data["index"].toInt();
    if (index < 0 || index >= playbackQueue_.size()) {
        return;
    }

    qInfo() << "Dequeue index:" << index;
    
    // Remove from queue
    QQueue<QString> newQueue;
    for (int i = 0; i < playbackQueue_.size(); ++i) {
        if (i != index) {
            newQueue.enqueue(playbackQueue_[i]);
        }
    }
    playbackQueue_ = newQueue;
    
    publishQueueChanged();
}

void MediaPlayerExtension::handleClearQueueCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    
    qInfo() << "Clear queue";
    playbackQueue_.clear();
    isQueueMode_ = false;
    publishQueueChanged();
}

void MediaPlayerExtension::playNext() {
    if (!mediaEngine_) {
        return;
    }

    if (!playbackQueue_.isEmpty()) {
        playFromQueue();
    } else {
        qInfo() << "Queue empty, stopping playback";
        mediaEngine_->stop();
    }
}

void MediaPlayerExtension::playFromQueue() {
    if (!mediaEngine_ || playbackQueue_.isEmpty()) {
        return;
    }

    QString uri = playbackQueue_.dequeue();
    currentTrackUri_ = uri;
    
    qInfo() << "Playing from queue:" << uri;
    
    mediaEngine_->setUri(uri);
    mediaEngine_->play();
    
    publishQueueChanged();
}

void MediaPlayerExtension::publishStateChanged() {
    if (!eventCap_ || !mediaEngine_) {
        return;
    }

    QVariantMap data;
    
    // State
    QString stateStr;
    switch (mediaEngine_->state()) {
        case IMediaEngine::State::Playing:
            stateStr = "playing";
            break;
        case IMediaEngine::State::Paused:
            stateStr = "paused";
            break;
        case IMediaEngine::State::Buffering:
            stateStr = "buffering";
            break;
        case IMediaEngine::State::Error:
            stateStr = "error";
            break;
        default:
            stateStr = "stopped";
            break;
    }
    data["state"] = stateStr;
    
    // Track info
    auto metadata = mediaEngine_->currentMetadata();
    QVariantMap trackInfo;
    trackInfo["uri"] = metadata.uri;
    trackInfo["title"] = metadata.title;
    trackInfo["artist"] = metadata.artist;
    trackInfo["album"] = metadata.album;
    trackInfo["duration"] = metadata.durationMs;
    data["track"] = trackInfo;
    
    eventCap_->emitEvent("state_changed", data);
}

void MediaPlayerExtension::publishPositionChanged(qint64 position) {
    if (!eventCap_ || !mediaEngine_) {
        return;
    }

    QVariantMap data;
    data["position"] = position;
    data["duration"] = mediaEngine_->duration();
    
    eventCap_->emitEvent("position_changed", data);
}

void MediaPlayerExtension::publishMetadataChanged() {
    if (!eventCap_ || !mediaEngine_) {
        return;
    }

    auto metadata = mediaEngine_->currentMetadata();
    
    QVariantMap data;
    data["uri"] = metadata.uri;
    data["title"] = metadata.title;
    data["artist"] = metadata.artist;
    data["album"] = metadata.album;
    data["albumArtist"] = metadata.albumArtist;
    data["genre"] = metadata.genre;
    data["year"] = metadata.year;
    data["trackNumber"] = metadata.trackNumber;
    data["duration"] = metadata.durationMs;
    data["bitrate"] = metadata.bitrate;
    data["codec"] = metadata.codec;
    data["artworkUrl"] = metadata.artworkUrl;
    
    eventCap_->emitEvent("metadata_changed", data);
}

void MediaPlayerExtension::publishQueueChanged() {
    if (!eventCap_) {
        return;
    }

    QVariantList queueList;
    for (const QString& uri : playbackQueue_) {
        QVariantMap item;
        item["uri"] = uri;
        queueList.append(item);
    }

    QVariantMap data;
    data["queue"] = queueList;
    data["size"] = playbackQueue_.size();
    
    eventCap_->emitEvent("queue_changed", data);
}

void MediaPlayerExtension::publishError(const QString& message) {
    if (!eventCap_) {
        return;
    }

    QVariantMap data;
    data["message"] = message;
    data["code"] = "engine_error";
    
    eventCap_->emitEvent("error", data);
}

}  // namespace media
}  // namespace extensions
}  // namespace opencardev::crankshaft
