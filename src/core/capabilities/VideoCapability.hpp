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
#include <QSize>
#include <QString>
#include <QUrl>
#include <functional>
#include "Capability.hpp"

namespace opencardev::crankshaft {
namespace core {
namespace capabilities {

/**
 * Video capability for video playback and camera access.
 *
 * Extensions with this capability can:
 * - Play video files and streams
 * - Access camera/webcam feeds
 * - Capture video frames
 * - Control video display parameters
 *
 * Extensions cannot directly access V4L2/GStreamer/FFmpeg.
 * Video operations are mediated for performance and security.
 */
class VideoCapability : public Capability {
  public:
    ~VideoCapability() override = default;

    QString id() const override { return "video"; }

    /**
     * Camera/video device information.
     */
    struct VideoDevice {
        QString id;
        QString name;
        QString type;  // "camera", "capture", "virtual"
        QList<QSize> supportedResolutions;
        QList<int> supportedFrameRates;
        bool isAvailable;
    };

    /**
     * Video playback state.
     */
    enum class PlaybackState { Stopped, Playing, Paused, Buffering, Error };

    /**
     * Video frame format.
     */
    enum class FrameFormat { RGB24, RGB32, YUV420, MJPEG, H264 };

    /**
     * Play video from URL or file path.
     * Video is displayed in extension's QML view.
     *
     * @param source URL or local file path
     * @param callback Called with playback ID or error
     */
    virtual void playVideo(const QUrl& source,
                           std::function<void(int playbackId, const QString& error)> callback) = 0;

    /**
     * Stop video playback.
     *
     * @param playbackId ID returned from playVideo()
     */
    virtual void stopVideo(int playbackId) = 0;

    /**
     * Pause video playback.
     *
     * @param playbackId ID returned from playVideo()
     */
    virtual void pauseVideo(int playbackId) = 0;

    /**
     * Resume video playback.
     *
     * @param playbackId ID returned from playVideo()
     */
    virtual void resumeVideo(int playbackId) = 0;

    /**
     * Seek to position in video.
     *
     * @param playbackId ID returned from playVideo()
     * @param positionMs Position in milliseconds
     */
    virtual void seekVideo(int playbackId, qint64 positionMs) = 0;

    /**
     * Get current video playback state.
     *
     * @param playbackId ID returned from playVideo()
     */
    virtual PlaybackState getPlaybackState(int playbackId) const = 0;

    /**
     * Get current video position.
     *
     * @param playbackId ID returned from playVideo()
     * @return Position in milliseconds
     */
    virtual qint64 getPosition(int playbackId) const = 0;

    /**
     * Get total video duration.
     *
     * @param playbackId ID returned from playVideo()
     * @return Duration in milliseconds
     */
    virtual qint64 getDuration(int playbackId) const = 0;

    /**
     * Get video resolution.
     *
     * @param playbackId ID returned from playVideo()
     * @return Video size (width x height)
     */
    virtual QSize getVideoSize(int playbackId) const = 0;

    /**
     * Get list of available video/camera devices.
     */
    virtual QList<VideoDevice> getVideoDevices() const = 0;

    /**
     * Open camera/video device for capturing.
     *
     * @param deviceId Device ID from getVideoDevices()
     * @param resolution Desired resolution
     * @param frameRate Desired frame rate
     * @param callback Called with capture ID or error
     */
    virtual void openCamera(const QString& deviceId, const QSize& resolution, int frameRate,
                            std::function<void(int captureId, const QString& error)> callback) = 0;

    /**
     * Close camera/video device.
     *
     * @param captureId ID returned from openCamera()
     */
    virtual void closeCamera(int captureId) = 0;

    /**
     * Start video recording from camera.
     *
     * @param captureId ID returned from openCamera()
     * @param outputPath File path to save recording
     * @param callback Called with recording ID or error
     */
    virtual void startRecording(
        int captureId, const QString& outputPath,
        std::function<void(int recordingId, const QString& error)> callback) = 0;

    /**
     * Stop video recording.
     *
     * @param recordingId ID returned from startRecording()
     */
    virtual void stopRecording(int recordingId) = 0;

    /**
     * Capture a single frame from camera.
     *
     * @param captureId ID returned from openCamera()
     * @param callback Called with frame data (format-dependent)
     */
    virtual void captureFrame(
        int captureId,
        std::function<void(const QByteArray& frameData, FrameFormat format)> callback) = 0;

    /**
     * Subscribe to video frames from camera.
     * Callback invoked for each new frame.
     *
     * @param captureId ID returned from openCamera()
     * @param callback Called with each frame
     * @return Subscription ID for unsubscribe
     */
    virtual int subscribeToFrames(
        int captureId,
        std::function<void(const QByteArray& frameData, FrameFormat format, qint64 timestamp)>
            callback) = 0;

    /**
     * Subscribe to video playback state changes.
     *
     * @param playbackId ID returned from playVideo()
     * @param callback Called when state changes
     * @return Subscription ID for unsubscribe
     */
    virtual int subscribeToPlaybackState(int playbackId,
                                         std::function<void(PlaybackState state)> callback) = 0;

    /**
     * Unsubscribe from frame or state notifications.
     *
     * @param subscriptionId ID returned from subscribe methods
     */
    virtual void unsubscribe(int subscriptionId) = 0;

    /**
     * Set video display brightness.
     *
     * @param playbackId ID returned from playVideo()
     * @param brightness Brightness level 0.0-1.0
     */
    virtual void setBrightness(int playbackId, float brightness) = 0;

    /**
     * Set video display contrast.
     *
     * @param playbackId ID returned from playVideo()
     * @param contrast Contrast level 0.0-2.0 (1.0 = normal)
     */
    virtual void setContrast(int playbackId, float contrast) = 0;
};

}  // namespace capabilities
}  // namespace core
}  // namespace opencardev::crankshaft
