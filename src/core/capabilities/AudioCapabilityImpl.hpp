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

#include <QHash>
#include "AudioCapability.hpp"

namespace opencardev::crankshaft::core {
class CapabilityManager;
}

namespace opencardev::crankshaft::core::capabilities {

class AudioCapabilityImpl : public AudioCapability {
  public:
    AudioCapabilityImpl(QString extension_id, core::CapabilityManager* manager);
    QString extensionId() const override;
    bool isValid() const override;
    void invalidate();

    void play(const QUrl& source, StreamType streamType,
              std::function<void(int, const QString&)> callback) override;
    void stop(int playbackId) override;
    void pause(int playbackId) override;
    void resume(int playbackId) override;
    void seek(int playbackId, qint64 positionMs) override;
    PlaybackState getPlaybackState(int playbackId) const override;
    qint64 getPosition(int playbackId) const override;
    qint64 getDuration(int playbackId) const override;
    void setVolume(int playbackId, float volume) override;
    float getVolume(int playbackId) const override;
    void setMasterVolume(float volume) override;
    float getMasterVolume() const override;
    void setMuted(bool muted) override;
    bool isMuted() const override;
    QList<AudioDevice> getOutputDevices() const override;
    QList<AudioDevice> getInputDevices() const override;
    void setOutputDevice(const QString& deviceId) override;
    void setInputDevice(const QString& deviceId) override;
    void startRecording(const QString& outputPath, int sampleRate, int channels,
                        std::function<void(int, const QString&)> callback) override;
    void stopRecording(int recordingId) override;
    int subscribeToPlaybackState(int playbackId,
                                 std::function<void(PlaybackState)> callback) override;
    void unsubscribe(int subscriptionId) override;

  private:
    QString extension_id_;
    core::CapabilityManager* manager_;
    bool is_valid_;
    int next_playback_id_;
    int next_subscription_id_ = 0;
    QHash<int, PlaybackState> playback_state_;
    QHash<int, float> playback_volume_;
    QHash<int, std::function<void(PlaybackState)>> state_subscribers_;
    float master_volume_;
    bool muted_;
};

inline std::shared_ptr<AudioCapability> createAudioCapabilityInstance(
    const QString& extensionId, core::CapabilityManager* mgr) {
    return std::static_pointer_cast<AudioCapability>(
        std::make_shared<AudioCapabilityImpl>(extensionId, mgr));
}

}  // namespace opencardev::crankshaft::core::capabilities
