/*
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
*/

#pragma once

#include <QList>
#include <QString>
#include <QUrl>
#include <functional>
#include "Capability.hpp"

namespace opencardev::crankshaft {
namespace core {
namespace capabilities {

/**
 * Audio capability for audio playback, recording, and routing.
 *
 * Extensions with this capability can:
 * - Play audio files and streams
 * - Record audio from microphone
 * - Control volume and audio routing
 * - Access audio device information
 * - Configure equaliser and audio effects
 *
 * Extensions cannot directly access ALSA/PulseAudio/PipeWire.
 * Audio operations are mediated to prevent conflicts between extensions.
 */
class AudioCapability : public Capability {
  public:
    virtual ~AudioCapability() = default;

    QString id() const override { return "audio"; }

    /**
     * Audio device information.
     */
    struct AudioDevice {
        QString id;
        QString name;
        QString type;  // "output", "input", "both"
        bool isDefault;
        int channels;
        QList<int> sampleRates;
    };

    /**
     * Playback state.
     */
    enum class PlaybackState { Stopped, Playing, Paused, Buffering, Error };

    /**
     * Audio stream type for routing priority.
     */
    enum class StreamType {
        Media,         // Music, videos
        Navigation,    // Turn-by-turn directions
        Notification,  // Alerts, notifications
        Phone,         // Phone calls
        SystemSound    // UI sounds
    };

    /**
     * Play audio from URL or file path.
     *
     * @param source URL or local file path
     * @param streamType Stream type for routing priority
     * @param callback Called with playback ID or error
     */
    virtual void play(const QUrl& source, StreamType streamType,
                      std::function<void(int playbackId, const QString& error)> callback) = 0;

    /**
     * Stop playback.
     *
     * @param playbackId ID returned from play()
     */
    virtual void stop(int playbackId) = 0;

    /**
     * Pause playback.
     *
     * @param playbackId ID returned from play()
     */
    virtual void pause(int playbackId) = 0;

    /**
     * Resume playback.
     *
     * @param playbackId ID returned from play()
     */
    virtual void resume(int playbackId) = 0;

    /**
     * Seek to position in playback.
     *
     * @param playbackId ID returned from play()
     * @param positionMs Position in milliseconds
     */
    virtual void seek(int playbackId, qint64 positionMs) = 0;

    /**
     * Get current playback state.
     *
     * @param playbackId ID returned from play()
     */
    virtual PlaybackState getPlaybackState(int playbackId) const = 0;

    /**
     * Get current playback position.
     *
     * @param playbackId ID returned from play()
     * @return Position in milliseconds
     */
    virtual qint64 getPosition(int playbackId) const = 0;

    /**
     * Get total duration of current media.
     *
     * @param playbackId ID returned from play()
     * @return Duration in milliseconds
     */
    virtual qint64 getDuration(int playbackId) const = 0;

    /**
     * Set volume for specific playback.
     *
     * @param playbackId ID returned from play()
     * @param volume Volume level 0.0-1.0
     */
    virtual void setVolume(int playbackId, float volume) = 0;

    /**
     * Get volume for specific playback.
     *
     * @param playbackId ID returned from play()
     * @return Volume level 0.0-1.0
     */
    virtual float getVolume(int playbackId) const = 0;

    /**
     * Set master volume.
     *
     * @param volume Volume level 0.0-1.0
     */
    virtual void setMasterVolume(float volume) = 0;

    /**
     * Get master volume.
     *
     * @return Volume level 0.0-1.0
     */
    virtual float getMasterVolume() const = 0;

    /**
     * Mute/unmute master audio.
     *
     * @param muted true to mute, false to unmute
     */
    virtual void setMuted(bool muted) = 0;

    /**
     * Check if master audio is muted.
     */
    virtual bool isMuted() const = 0;

    /**
     * Get list of available audio output devices.
     */
    virtual QList<AudioDevice> getOutputDevices() const = 0;

    /**
     * Get list of available audio input devices.
     */
    virtual QList<AudioDevice> getInputDevices() const = 0;

    /**
     * Set default audio output device.
     *
     * @param deviceId Device ID from getOutputDevices()
     */
    virtual void setOutputDevice(const QString& deviceId) = 0;

    /**
     * Set default audio input device.
     *
     * @param deviceId Device ID from getInputDevices()
     */
    virtual void setInputDevice(const QString& deviceId) = 0;

    /**
     * Start audio recording.
     *
     * @param outputPath File path to save recording
     * @param sampleRate Sample rate in Hz (e.g., 44100)
     * @param channels Number of channels (1=mono, 2=stereo)
     * @param callback Called with recording ID or error
     */
    virtual void startRecording(
        const QString& outputPath, int sampleRate, int channels,
        std::function<void(int recordingId, const QString& error)> callback) = 0;

    /**
     * Stop audio recording.
     *
     * @param recordingId ID returned from startRecording()
     */
    virtual void stopRecording(int recordingId) = 0;

    /**
     * Subscribe to playback state changes.
     *
     * @param playbackId ID returned from play()
     * @param callback Called when state changes
     * @return Subscription ID for unsubscribe
     */
    virtual int subscribeToPlaybackState(int playbackId,
                                         std::function<void(PlaybackState state)> callback) = 0;

    /**
     * Unsubscribe from playback state notifications.
     *
     * @param subscriptionId ID returned from subscribeToPlaybackState
     */
    virtual void unsubscribe(int subscriptionId) = 0;
};

}  // namespace capabilities
}  // namespace core
}  // namespace opencardev::crankshaft
