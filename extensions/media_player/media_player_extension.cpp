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
#include "../../src/core/event_bus.hpp"
#include "../../src/core/config/ConfigManager.hpp"
#include "../../src/core/config/ConfigTypes.hpp"
#include <QDebug>

namespace opencardev::crankshaft {
namespace extensions {
namespace media {

bool MediaPlayerExtension::initialize() {
    qInfo() << "Initializing Media Player extension...";
    setupEventHandlers();
    return true;
}

void MediaPlayerExtension::start() {
    qInfo() << "Starting Media Player extension...";
}

void MediaPlayerExtension::stop() {
    qInfo() << "Stopping Media Player extension...";
}

void MediaPlayerExtension::cleanup() {
    qInfo() << "Cleaning up Media Player extension...";
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
    equalizerPresetItem.properties["options"] = QStringList{"Flat", "Pop", "Rock", "Jazz", "Classical", "Bass Boost", "Treble Boost", "Custom"};
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
    libraryPathsItem.properties["options"] = QStringList{"/media/music", "/media/usb", "/media/sdcard"};
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

void MediaPlayerExtension::setupEventHandlers() {
    if (!event_bus_) {
        qWarning() << "Event bus not available";
        return;
    }

    event_bus_->subscribe("media.play", [this](const QVariantMap& data) {
        handlePlayCommand(data);
    });

    event_bus_->subscribe("media.pause", [this](const QVariantMap& data) {
        handlePauseCommand(data);
    });

    event_bus_->subscribe("media.stop", [this](const QVariantMap& data) {
        handleStopCommand(data);
    });

    event_bus_->subscribe("media.next", [this](const QVariantMap& data) {
        handleNextCommand(data);
    });

    event_bus_->subscribe("media.previous", [this](const QVariantMap& data) {
        handlePreviousCommand(data);
    });
}

void MediaPlayerExtension::handlePlayCommand(const QVariantMap& data) {
    qDebug() << "Play command received:" << data;
    // TODO: Implement media playback logic
}

void MediaPlayerExtension::handlePauseCommand(const QVariantMap& data) {
    qDebug() << "Pause command received:" << data;
    // TODO: Implement pause logic
}

void MediaPlayerExtension::handleStopCommand(const QVariantMap& data) {
    qDebug() << "Stop command received:" << data;
    // TODO: Implement stop logic
}

void MediaPlayerExtension::handleNextCommand(const QVariantMap& data) {
    qDebug() << "Next command received:" << data;
    // TODO: Implement next track logic
}

void MediaPlayerExtension::handlePreviousCommand(const QVariantMap& data) {
    qDebug() << "Previous command received:" << data;
    // TODO: Implement previous track logic
}

}  // namespace media
}  // namespace extensions
}  // namespace opencardev::crankshaft
