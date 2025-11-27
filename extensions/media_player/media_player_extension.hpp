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

#pragma once

#include <memory>
#include <QQueue>
#include "../../src/core/capabilities/EventCapability.hpp"
#include "../../src/extensions/extension.hpp"
#include "IMediaEngine.hpp"

namespace opencardev::crankshaft {
namespace extensions {
namespace media {

class MediaPlayerExtension : public Extension {
  public:
    MediaPlayerExtension() = default;
    ~MediaPlayerExtension() override = default;

    // Lifecycle methods
    bool initialize() override;
    void start() override;
    void stop() override;
    void cleanup() override;

    // Metadata
    QString id() const override { return "media_player"; }
    QString name() const override { return "Media Player"; }
    QString version() const override { return "1.0.0"; }
    ExtensionType type() const override { return ExtensionType::UI; }

    // Configuration
    void registerConfigItems(core::config::ConfigManager* manager) override;

  private:
    void setupEventHandlers();
    void setupEngineCallbacks();
    
    // Command handlers
    void handlePlayCommand(const QVariantMap& data);
    void handlePauseCommand(const QVariantMap& data);
    void handleStopCommand(const QVariantMap& data);
    void handleNextCommand(const QVariantMap& data);
    void handlePreviousCommand(const QVariantMap& data);
    void handleSeekCommand(const QVariantMap& data);
    void handleSetVolumeCommand(const QVariantMap& data);
    void handleSetMutedCommand(const QVariantMap& data);
    void handleEnqueueCommand(const QVariantMap& data);
    void handleDequeueCommand(const QVariantMap& data);
    void handleClearQueueCommand(const QVariantMap& data);
    
    // Queue management
    void playNext();
    void playFromQueue();
    
    // Event publishing helpers
    void publishStateChanged();
    void publishPositionChanged(qint64 position);
    void publishMetadataChanged();
    void publishQueueChanged();
    void publishError(const QString& message);

    std::shared_ptr<core::capabilities::EventCapability> eventCap_;
    std::unique_ptr<IMediaEngine> mediaEngine_;
    
    // Playback queue
    QQueue<QString> playbackQueue_;
    QString currentTrackUri_;
    bool isQueueMode_{false};
};

}  // namespace media
}  // namespace extensions
}  // namespace opencardev::crankshaft
