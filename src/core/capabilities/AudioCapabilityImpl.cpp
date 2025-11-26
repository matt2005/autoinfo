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
#include "AudioCapabilityImpl.hpp"
#include "CapabilityManager.hpp"

using namespace opencardev::crankshaft::core::capabilities;
using opencardev::crankshaft::core::CapabilityManager;

AudioCapabilityImpl::AudioCapabilityImpl(QString extension_id, CapabilityManager* manager)
        : extension_id_(std::move(extension_id)),
      manager_(manager),
      is_valid_(true),
      next_playback_id_(1),
            master_volume_(1.0F),
      muted_(false) {}

auto AudioCapabilityImpl::extensionId() const -> QString { return extension_id_; }
auto AudioCapabilityImpl::isValid() const -> bool { return is_valid_; }
void AudioCapabilityImpl::invalidate() {
    is_valid_ = false;
}

void AudioCapabilityImpl::play(const QUrl& source, StreamType streamType,
                               std::function<void(int, const QString&)> callback) {
    if (!is_valid_) {
        callback(-1, QStringLiteral("invalid capability"));
        return;
    }
    Q_UNUSED(streamType);
    manager_->logCapabilityUsage(extension_id_, "audio", "play", source.toString());
    const int playback_id = next_playback_id_++;
    playback_state_[playback_id] = PlaybackState::Playing;
    playback_volume_[playback_id] = 1.0F;
    callback(playback_id, QString());
}

void AudioCapabilityImpl::stop(int playbackId) {
    playback_state_[playbackId] = PlaybackState::Stopped;
    manager_->logCapabilityUsage(extension_id_, "audio", "stop", QString::number(playbackId));
}
void AudioCapabilityImpl::pause(int playbackId) {
    playback_state_[playbackId] = PlaybackState::Paused;
    manager_->logCapabilityUsage(extension_id_, "audio", "pause", QString::number(playbackId));
}
void AudioCapabilityImpl::resume(int playbackId) {
    playback_state_[playbackId] = PlaybackState::Playing;
    manager_->logCapabilityUsage(extension_id_, "audio", "resume", QString::number(playbackId));
}
void AudioCapabilityImpl::seek(int playbackId, qint64 positionMs) {
    Q_UNUSED(positionMs);
    manager_->logCapabilityUsage(extension_id_, "audio", "seek", QString::number(playbackId));
}
auto AudioCapabilityImpl::getPlaybackState(int playbackId) const -> AudioCapability::PlaybackState {
    return playback_state_.value(playbackId, PlaybackState::Stopped);
}
auto AudioCapabilityImpl::getPosition(int playbackId) const -> qint64 {
    Q_UNUSED(playbackId);
    return 0;
}
auto AudioCapabilityImpl::getDuration(int playbackId) const -> qint64 {
    Q_UNUSED(playbackId);
    return 0;
}
void AudioCapabilityImpl::setVolume(int playbackId, float volume) { playback_volume_[playbackId] = volume; }
auto AudioCapabilityImpl::getVolume(int playbackId) const -> float { return playback_volume_.value(playbackId, 1.0F); }
void AudioCapabilityImpl::setMasterVolume(float volume) {
    master_volume_ = volume;
}
auto AudioCapabilityImpl::getMasterVolume() const -> float { return master_volume_; }
void AudioCapabilityImpl::setMuted(bool muted) {
    muted_ = muted;
}
auto AudioCapabilityImpl::isMuted() const -> bool { return muted_; }
auto AudioCapabilityImpl::getOutputDevices() const -> QList<AudioCapability::AudioDevice> { return {}; }
auto AudioCapabilityImpl::getInputDevices() const -> QList<AudioCapabilityImpl::AudioDevice> { return {}; }
void AudioCapabilityImpl::setOutputDevice(const QString& deviceId) {
    Q_UNUSED(deviceId);
}
void AudioCapabilityImpl::setInputDevice(const QString& deviceId) {
    Q_UNUSED(deviceId);
}
void AudioCapabilityImpl::startRecording(const QString& outputPath, int sampleRate, int channels,
                                         std::function<void(int, const QString&)> callback) {
    Q_UNUSED(outputPath);
    Q_UNUSED(sampleRate);
    Q_UNUSED(channels);
    callback(-1, QStringLiteral("audio recording not available"));
}
void AudioCapabilityImpl::stopRecording(int recordingId) {
    Q_UNUSED(recordingId);
}
auto AudioCapabilityImpl::subscribeToPlaybackState(int playbackId,
                                                  std::function<void(PlaybackState)> callback) -> int {
    const int sub_id = ++next_subscription_id_;
    state_subscribers_[sub_id] = callback;
    Q_UNUSED(playbackId);
    return sub_id;
}
void AudioCapabilityImpl::unsubscribe(int subscriptionId) {
    state_subscribers_.remove(subscriptionId);
}
