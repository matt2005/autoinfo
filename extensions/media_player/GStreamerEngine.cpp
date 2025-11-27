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

#include "GStreamerEngine.hpp"
#include <QDebug>
#include <gst/gst.h>

namespace opencardev::crankshaft {
namespace extensions {
namespace media {

GStreamerEngine::GStreamerEngine(QObject* parent)
    : IMediaEngine(parent), positionTimer_(new QTimer(this)), busMessageTimer_(new QTimer(this)) {
    
    // Position updates every 250ms during playback
    positionTimer_->setInterval(250);
    connect(positionTimer_, &QTimer::timeout, this, &GStreamerEngine::updatePosition);

    // Bus message polling every 50ms
    busMessageTimer_->setInterval(50);
    connect(busMessageTimer_, &QTimer::timeout, this, &GStreamerEngine::checkForMessages);
}

GStreamerEngine::~GStreamerEngine() {
    shutdown();
}

bool GStreamerEngine::initialize() {
    qInfo() << "Initialising GStreamer media engine...";

    // Initialise GStreamer
    GError* error = nullptr;
    if (!gst_init_check(nullptr, nullptr, &error)) {
        qCritical() << "Failed to initialise GStreamer:" 
                    << (error ? error->message : "Unknown error");
        if (error)
            g_error_free(error);
        return false;
    }

    qInfo() << "GStreamer version:" 
            << GST_VERSION_MAJOR << "." 
            << GST_VERSION_MINOR << "." 
            << GST_VERSION_MICRO;

    // Detect hardware capabilities
    hardwareDecodeAvailable_ = detectHardwareDecoders();
    
    // Set up capabilities
    capabilities_.supportsVideo = true;
    capabilities_.supportsGapless = true;
    capabilities_.supportsHardwareDecode = hardwareDecodeAvailable_;
    capabilities_.supportsStreaming = true;
    capabilities_.supportsSeek = true;
    capabilities_.supportedFormats = {
        "mp3", "flac", "wav", "ogg", "opus", "aac", "m4a", "wma",
        "mp4", "mkv", "avi", "webm", "mov", "flv"
    };

    // Create GStreamer discoverer for metadata extraction
    discoverer_ = gst_discoverer_new(5 * GST_SECOND, &error);
    if (!discoverer_) {
        qWarning() << "Failed to create GStreamer discoverer:" 
                   << (error ? error->message : "Unknown error");
        if (error)
            g_error_free(error);
    }

    setupPipeline();
    busMessageTimer_->start();

    qInfo() << "GStreamer engine initialised successfully";
    qInfo() << "Hardware decode available:" << hardwareDecodeAvailable_;
    
    return true;
}

void GStreamerEngine::shutdown() {
    if (isShutdown_) {
        return;
    }
    
    qInfo() << "Shutting down GStreamer engine...";
    isShutdown_ = true;

    positionTimer_->stop();
    busMessageTimer_->stop();

    cleanupPipeline();

    if (discoverer_) {
        gst_object_unref(discoverer_);
        discoverer_ = nullptr;
    }

    // Note: gst_deinit() should NOT be called in production code.
    // GStreamer manages its own cleanup. Calling gst_deinit() can cause
    // crashes if called multiple times or if other parts of the application
    // are still using GStreamer.
}

void GStreamerEngine::setupPipeline() {
    // Create playbin element (high-level playback element)
    playbin_ = gst_element_factory_make("playbin", "playbin");
    if (!playbin_) {
        qCritical() << "Failed to create playbin element";
        return;
    }

    pipeline_ = playbin_;

    // Get bus for messages
    bus_ = gst_element_get_bus(pipeline_);

    // Connect signals
    g_signal_connect(playbin_, "source-setup", G_CALLBACK(onSourceSetup), this);
    g_signal_connect(playbin_, "about-to-finish", G_CALLBACK(onAboutToFinish), this);

    // Set initial volume
    g_object_set(playbin_, "volume", currentVolume_ / 100.0, nullptr);
    g_object_set(playbin_, "mute", isMuted_, nullptr);

    qInfo() << "GStreamer pipeline created successfully";
}

void GStreamerEngine::cleanupPipeline() {
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
        playbin_ = nullptr;
    }

    if (bus_) {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }
}

bool GStreamerEngine::setUri(const QString& uri) {
    if (!playbin_) {
        qWarning() << "Cannot set URI: playbin not initialised";
        return false;
    }

    qInfo() << "Setting URI:" << uri;

    // Stop current playback
    gst_element_set_state(playbin_, GST_STATE_NULL);

    currentUri_ = uri;
    currentMetadata_ = TrackMetadata();
    currentMetadata_.uri = uri;
    cachedDuration_ = -1;

    // Set URI on playbin
    g_object_set(playbin_, "uri", uri.toUtf8().constData(), nullptr);

    // Preload to ready state
    setState(GST_STATE_READY);

    return true;
}

bool GStreamerEngine::play() {
    if (!playbin_) {
        qWarning() << "Cannot play: playbin not initialised";
        return false;
    }

    qInfo() << "Starting playback";
    setState(GST_STATE_PLAYING);
    positionTimer_->start();

    return true;
}

bool GStreamerEngine::pause() {
    if (!playbin_) {
        qWarning() << "Cannot pause: playbin not initialised";
        return false;
    }

    qInfo() << "Pausing playback";
    setState(GST_STATE_PAUSED);
    positionTimer_->stop();

    return true;
}

bool GStreamerEngine::stop() {
    if (!playbin_) {
        qWarning() << "Cannot stop: playbin not initialised";
        return false;
    }

    qInfo() << "Stopping playback";
    setState(GST_STATE_NULL);
    positionTimer_->stop();

    currentUri_.clear();
    currentMetadata_ = TrackMetadata();
    cachedDuration_ = -1;

    return true;
}

bool GStreamerEngine::seek(qint64 positionMs) {
    if (!playbin_) {
        qWarning() << "Cannot seek: playbin not initialised";
        return false;
    }

    qint64 positionNs = positionMs * GST_MSECOND;

    qDebug() << "Seeking to position:" << positionMs << "ms";

    if (!gst_element_seek_simple(playbin_, GST_FORMAT_TIME,
                                  static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                                  positionNs)) {
        qWarning() << "Seek failed";
        return false;
    }

    return true;
}

IMediaEngine::State GStreamerEngine::state() const {
    return currentState_;
}

qint64 GStreamerEngine::position() const {
    if (!playbin_) {
        return -1;
    }

    gint64 pos = -1;
    if (gst_element_query_position(playbin_, GST_FORMAT_TIME, &pos)) {
        return pos / GST_MSECOND;
    }

    return -1;
}

qint64 GStreamerEngine::duration() const {
    if (cachedDuration_ >= 0) {
        return cachedDuration_;
    }

    if (!playbin_) {
        return -1;
    }

    gint64 dur = -1;
    if (gst_element_query_duration(playbin_, GST_FORMAT_TIME, &dur)) {
        return dur / GST_MSECOND;
    }

    return -1;
}

bool GStreamerEngine::setVolume(int percent) {
    if (!playbin_) {
        return false;
    }

    currentVolume_ = qBound(0, percent, 100);
    g_object_set(playbin_, "volume", currentVolume_ / 100.0, nullptr);

    qDebug() << "Volume set to:" << currentVolume_ << "%";
    return true;
}

int GStreamerEngine::volume() const {
    return currentVolume_;
}

bool GStreamerEngine::setMuted(bool muted) {
    if (!playbin_) {
        return false;
    }

    isMuted_ = muted;
    g_object_set(playbin_, "mute", isMuted_ ? TRUE : FALSE, nullptr);

    qDebug() << "Mute set to:" << isMuted_;
    return true;
}

bool GStreamerEngine::isMuted() const {
    return isMuted_;
}

IMediaEngine::TrackMetadata GStreamerEngine::currentMetadata() const {
    return currentMetadata_;
}

bool GStreamerEngine::extractMetadata(const QString& uri, TrackMetadata& out) {
    if (!discoverer_) {
        qWarning() << "Discoverer not available for metadata extraction";
        return false;
    }

    qDebug() << "Extracting metadata from:" << uri;

    GError* error = nullptr;
    GstDiscovererInfo* info = gst_discoverer_discover_uri(
        discoverer_, uri.toUtf8().constData(), &error);

    if (error) {
        qWarning() << "Metadata extraction failed:" << error->message;
        g_error_free(error);
        return false;
    }

    if (!info) {
        qWarning() << "No discoverer info returned";
        return false;
    }

    out.uri = uri;

    // Extract duration
    GstClockTime dur = gst_discoverer_info_get_duration(info);
    if (dur != GST_CLOCK_TIME_NONE) {
        out.durationMs = dur / GST_MSECOND;
    }

    // Extract tags
    const GstTagList* tags = gst_discoverer_info_get_tags(info);
    if (tags) {
        gchar* str = nullptr;
        
        if (gst_tag_list_get_string(tags, GST_TAG_TITLE, &str)) {
            out.title = QString::fromUtf8(str);
            g_free(str);
        }
        
        if (gst_tag_list_get_string(tags, GST_TAG_ARTIST, &str)) {
            out.artist = QString::fromUtf8(str);
            g_free(str);
        }
        
        if (gst_tag_list_get_string(tags, GST_TAG_ALBUM, &str)) {
            out.album = QString::fromUtf8(str);
            g_free(str);
        }
        
        if (gst_tag_list_get_string(tags, GST_TAG_ALBUM_ARTIST, &str)) {
            out.albumArtist = QString::fromUtf8(str);
            g_free(str);
        }
        
        if (gst_tag_list_get_string(tags, GST_TAG_GENRE, &str)) {
            out.genre = QString::fromUtf8(str);
            g_free(str);
        }

        guint track = 0;
        if (gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &track)) {
            out.trackNumber = track;
        }

        GDate* date = nullptr;
        if (gst_tag_list_get_date(tags, GST_TAG_DATE, &date)) {
            out.year = g_date_get_year(date);
            g_date_free(date);
        }

        guint bitrate = 0;
        if (gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrate)) {
            out.bitrate = bitrate / 1000;  // Convert to kbps
        }
    }

    // Determine media type
    GList* streamList = gst_discoverer_info_get_stream_list(info);
    bool hasVideo = false;
    bool hasAudio = false;

    for (GList* l = streamList; l != nullptr; l = l->next) {
        GstDiscovererStreamInfo* streamInfo = static_cast<GstDiscovererStreamInfo*>(l->data);
        
        if (GST_IS_DISCOVERER_VIDEO_INFO(streamInfo)) {
            hasVideo = true;
        } else if (GST_IS_DISCOVERER_AUDIO_INFO(streamInfo)) {
            hasAudio = true;
        }
    }

    gst_discoverer_stream_info_list_free(streamList);

    if (hasVideo) {
        out.type = MediaType::Video;
    } else if (hasAudio) {
        out.type = MediaType::Audio;
    }

    gst_discoverer_info_unref(info);

    qDebug() << "Metadata extracted:" << out.title << "-" << out.artist;
    return true;
}

IMediaEngine::Capabilities GStreamerEngine::capabilities() const {
    return capabilities_;
}

void GStreamerEngine::updatePosition() {
    qint64 pos = position();
    if (pos >= 0) {
        emit positionChanged(pos);
    }

    // Update duration if not cached
    if (cachedDuration_ < 0) {
        qint64 dur = duration();
        if (dur >= 0) {
            cachedDuration_ = dur;
            emit durationChanged(dur);
        }
    }
}

void GStreamerEngine::checkForMessages() {
    if (!bus_) {
        return;
    }

    GstMessage* msg;
    while ((msg = gst_bus_pop(bus_)) != nullptr) {
        handleBusMessage(msg);
        gst_message_unref(msg);
    }
}

void GStreamerEngine::handleBusMessage(GstMessage* msg) {
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError* error = nullptr;
            gchar* debug = nullptr;
            gst_message_parse_error(msg, &error, &debug);
            
            QString errorMsg = QString::fromUtf8(error->message);
            qCritical() << "GStreamer error:" << errorMsg;
            qDebug() << "Debug info:" << (debug ? debug : "none");
            
            emit this->error(errorMsg);
            currentState_ = State::Error;
            emit stateChanged(currentState_);
            
            g_error_free(error);
            g_free(debug);
            break;
        }

        case GST_MESSAGE_EOS:
            qInfo() << "End of stream";
            currentState_ = State::Stopped;
            emit endOfStream();
            emit stateChanged(currentState_);
            positionTimer_->stop();
            break;

        case GST_MESSAGE_STATE_CHANGED: {
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(playbin_)) {
                GstState oldState, newState, pending;
                gst_message_parse_state_changed(msg, &oldState, &newState, &pending);
                
                State engineState = gstStateToEngineState(newState);
                if (engineState != currentState_) {
                    currentState_ = engineState;
                    emit stateChanged(currentState_);
                    
                    qDebug() << "State changed to:" << static_cast<int>(currentState_);
                }
            }
            break;
        }

        case GST_MESSAGE_TAG: {
            GstTagList* tags = nullptr;
            gst_message_parse_tag(msg, &tags);
            if (tags) {
                extractMetadataFromTags(tags);
                gst_tag_list_unref(tags);
            }
            break;
        }

        case GST_MESSAGE_BUFFERING: {
            gint percent = 0;
            gst_message_parse_buffering(msg, &percent);
            emit bufferingChanged(percent);
            
            if (percent < 100) {
                if (currentState_ != State::Buffering) {
                    currentState_ = State::Buffering;
                    emit stateChanged(currentState_);
                }
            }
            break;
        }

        case GST_MESSAGE_DURATION_CHANGED: {
            cachedDuration_ = -1;  // Force re-query
            qint64 dur = duration();
            if (dur >= 0) {
                cachedDuration_ = dur;
                emit durationChanged(dur);
            }
            break;
        }

        default:
            break;
    }
}

void GStreamerEngine::setState(GstState gstState) {
    if (playbin_) {
        gst_element_set_state(playbin_, gstState);
    }
}

IMediaEngine::State GStreamerEngine::gstStateToEngineState(GstState gstState) const {
    switch (gstState) {
        case GST_STATE_PLAYING:
            return State::Playing;
        case GST_STATE_PAUSED:
            return State::Paused;
        case GST_STATE_NULL:
        case GST_STATE_READY:
        default:
            return State::Stopped;
    }
}

void GStreamerEngine::extractMetadataFromTags(GstTagList* tags) {
    gchar* str = nullptr;
    bool changed = false;

    if (gst_tag_list_get_string(tags, GST_TAG_TITLE, &str)) {
        currentMetadata_.title = QString::fromUtf8(str);
        g_free(str);
        changed = true;
    }

    if (gst_tag_list_get_string(tags, GST_TAG_ARTIST, &str)) {
        currentMetadata_.artist = QString::fromUtf8(str);
        g_free(str);
        changed = true;
    }

    if (gst_tag_list_get_string(tags, GST_TAG_ALBUM, &str)) {
        currentMetadata_.album = QString::fromUtf8(str);
        g_free(str);
        changed = true;
    }

    if (changed) {
        qDebug() << "Metadata updated from stream tags";
        emit metadataChanged(currentMetadata_);
    }
}

bool GStreamerEngine::detectHardwareDecoders() {
    // Check for V4L2 hardware decoders (Raspberry Pi)
    GstElementFactory* v4l2H264 = gst_element_factory_find("v4l2h264dec");
    GstElementFactory* v4l2Mpeg2 = gst_element_factory_find("v4l2mpeg2dec");

    bool hasV4l2 = (v4l2H264 != nullptr || v4l2Mpeg2 != nullptr);

    if (v4l2H264)
        gst_object_unref(v4l2H264);
    if (v4l2Mpeg2)
        gst_object_unref(v4l2Mpeg2);

    return hasV4l2;
}

// Static callbacks
void GStreamerEngine::onSourceSetup(GstElement* playbin, GstElement* source, gpointer userData) {
    Q_UNUSED(playbin);
    Q_UNUSED(source);
    Q_UNUSED(userData);
    
    // Can configure source element here if needed (e.g., network timeouts)
    qDebug() << "Source element set up";
}

void GStreamerEngine::onAboutToFinish(GstElement* playbin, gpointer userData) {
    Q_UNUSED(playbin);
    
    auto* engine = static_cast<GStreamerEngine*>(userData);
    qInfo() << "About to finish - ready for gapless transition";
    
    // Emit signal so MediaService can queue next track
    // For true gapless, MediaService should call setUri() on next track here
    emit engine->endOfStream();
}

void GStreamerEngine::onPadAdded(GstElement* element, GstPad* pad, gpointer userData) {
    Q_UNUSED(element);
    Q_UNUSED(pad);
    Q_UNUSED(userData);
    
    qDebug() << "Pad added to element";
}

}  // namespace media
}  // namespace extensions
}  // namespace opencardev::crankshaft
