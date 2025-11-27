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

#include "IMediaEngine.hpp"
#include <QTimer>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

namespace opencardev::crankshaft {
namespace extensions {
namespace media {

/**
 * GStreamer-based implementation of IMediaEngine.
 * 
 * Provides audio and video playback using GStreamer 1.x pipelines.
 * Supports hardware acceleration on Raspberry Pi via V4L2.
 * 
 * Pipeline Structure:
 * - Audio: uridecodebin → audioconvert → audioresample → autoaudiosink
 * - Video: uridecodebin → videoconvert → videoscale → autovideosink
 * 
 * Features:
 * - Gapless playback via about-to-finish signal
 * - Hardware decode on Raspberry Pi (v4l2h264dec)
 * - Metadata extraction via GstDiscoverer
 * - PipeWire/PulseAudio support
 */
class GStreamerEngine : public IMediaEngine {
    Q_OBJECT

  public:
    explicit GStreamerEngine(QObject* parent = nullptr);
    ~GStreamerEngine() override;

    // IMediaEngine implementation
    bool initialize() override;
    void shutdown() override;

    bool setUri(const QString& uri) override;
    bool play() override;
    bool pause() override;
    bool stop() override;
    bool seek(qint64 positionMs) override;

    State state() const override;
    qint64 position() const override;
    qint64 duration() const override;

    bool setVolume(int percent) override;
    int volume() const override;
    bool setMuted(bool muted) override;
    bool isMuted() const override;

    TrackMetadata currentMetadata() const override;
    bool extractMetadata(const QString& uri, TrackMetadata& out) override;

    Capabilities capabilities() const override;

  private slots:
    void updatePosition();
    void checkForMessages();

  private:
    // GStreamer callbacks (must be static for C API)
    static void onSourceSetup(GstElement* playbin, GstElement* source, gpointer userData);
    static void onAboutToFinish(GstElement* playbin, gpointer userData);
    static void onPadAdded(GstElement* element, GstPad* pad, gpointer userData);

    // Internal helpers
    void setupPipeline();
    void cleanupPipeline();
    void handleBusMessage(GstMessage* msg);
    void setState(GstState gstState);
    State gstStateToEngineState(GstState gstState) const;
    void extractMetadataFromTags(GstTagList* tags);
    bool detectHardwareDecoders();

    // GStreamer objects
    GstElement* pipeline_{nullptr};
    GstElement* playbin_{nullptr};
    GstElement* audioSink_{nullptr};
    GstElement* videoSink_{nullptr};
    GstBus* bus_{nullptr};
    GstDiscoverer* discoverer_{nullptr};

    // State
    State currentState_{State::Stopped};
    TrackMetadata currentMetadata_;
    QString currentUri_;
    int currentVolume_{75};
    bool isMuted_{false};
    qint64 cachedDuration_{-1};
    bool isShutdown_{false};

    // Timers
    QTimer* positionTimer_;
    QTimer* busMessageTimer_;

    // Capabilities
    Capabilities capabilities_;
    bool hardwareDecodeAvailable_{false};
};

}  // namespace media
}  // namespace extensions
}  // namespace opencardev::crankshaft
