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

#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>

namespace opencardev::crankshaft {
namespace extensions {
namespace media {

/**
 * Abstract interface for media playback engines.
 * 
 * This interface abstracts the underlying media engine (GStreamer, VLC, FFmpeg, etc.)
 * allowing the implementation to be swapped without affecting consumers.
 * 
 * All engines must implement this contract to work with MediaService.
 */
class IMediaEngine : public QObject {
    Q_OBJECT

  public:
    enum class State {
        Stopped,    ///< No media loaded or playback stopped
        Playing,    ///< Currently playing
        Paused,     ///< Paused
        Buffering,  ///< Loading/buffering media
        Error       ///< Error state
    };
    Q_ENUM(State)

    enum class MediaType {
        Audio,   ///< Audio only
        Video,   ///< Video (with or without audio)
        Unknown  ///< Unknown or not yet determined
    };
    Q_ENUM(MediaType)

    struct Capabilities {
        bool supportsVideo{false};           ///< Can play video
        bool supportsGapless{false};         ///< Supports gapless playback
        bool supportsHardwareDecode{false};  ///< Hardware video decoding available
        bool supportsStreaming{false};       ///< Can play network streams
        bool supportsSeek{false};            ///< Seeking supported
        QStringList supportedFormats;        ///< List of supported file extensions
    };

    struct TrackMetadata {
        QString uri;           ///< Source URI (file://, http://, etc.)
        QString title;         ///< Track title
        QString artist;        ///< Artist name
        QString album;         ///< Album name
        QString albumArtist;   ///< Album artist
        QString genre;         ///< Genre
        QString artworkUrl;    ///< Artwork URL or file path
        qint64 durationMs{0};  ///< Duration in milliseconds
        int bitrate{0};        ///< Bitrate in kbps
        int year{0};           ///< Release year
        int trackNumber{0};    ///< Track number in album
        QString codec;         ///< Codec name
        MediaType type{MediaType::Unknown};
    };

    virtual ~IMediaEngine() = default;

    // ========== Lifecycle ==========

    /**
     * Initialise the media engine.
     * Called once during extension startup.
     * 
     * @return true if initialisation successful
     */
    virtual bool initialize() = 0;

    /**
     * Shut down the media engine.
     * Called during extension cleanup.
     * Should release all resources.
     */
    virtual void shutdown() = 0;

    // ========== Playback Control ==========

    /**
     * Set the media URI to play.
     * Supports file://, http://, https://, rtsp://, and custom schemes.
     * 
     * @param uri Media URI
     * @return true if URI accepted and loading started
     */
    virtual bool setUri(const QString& uri) = 0;

    /**
     * Start or resume playback.
     * 
     * @return true if playback started
     */
    virtual bool play() = 0;

    /**
     * Pause playback.
     * 
     * @return true if playback paused
     */
    virtual bool pause() = 0;

    /**
     * Stop playback and unload media.
     * 
     * @return true if stopped successfully
     */
    virtual bool stop() = 0;

    /**
     * Seek to a specific position.
     * 
     * @param positionMs Position in milliseconds
     * @return true if seek successful
     */
    virtual bool seek(qint64 positionMs) = 0;

    // ========== State Queries ==========

    /**
     * Get current playback state.
     * 
     * @return Current state
     */
    virtual State state() const = 0;

    /**
     * Get current playback position.
     * 
     * @return Position in milliseconds, or -1 if unknown
     */
    virtual qint64 position() const = 0;

    /**
     * Get media duration.
     * 
     * @return Duration in milliseconds, or -1 if unknown
     */
    virtual qint64 duration() const = 0;

    // ========== Volume & Output ==========

    /**
     * Set volume level.
     * 
     * @param percent Volume (0-100)
     * @return true if volume set successfully
     */
    virtual bool setVolume(int percent) = 0;

    /**
     * Get current volume level.
     * 
     * @return Volume (0-100)
     */
    virtual int volume() const = 0;

    /**
     * Set muted state.
     * 
     * @param muted true to mute, false to unmute
     * @return true if mute state changed
     */
    virtual bool setMuted(bool muted) = 0;

    /**
     * Get muted state.
     * 
     * @return true if muted
     */
    virtual bool isMuted() const = 0;

    // ========== Metadata ==========

    /**
     * Get metadata for currently loaded media.
     * 
     * @return Current track metadata
     */
    virtual TrackMetadata currentMetadata() const = 0;

    /**
     * Extract metadata from a URI without loading it.
     * This may be a blocking operation.
     * 
     * @param uri Media URI
     * @param out Output metadata struct
     * @return true if metadata extracted successfully
     */
    virtual bool extractMetadata(const QString& uri, TrackMetadata& out) = 0;

    // ========== Capabilities ==========

    /**
     * Query engine capabilities.
     * 
     * @return Capabilities struct
     */
    virtual Capabilities capabilities() const = 0;

  signals:
    /**
     * Emitted when playback state changes.
     * 
     * @param newState New playback state
     */
    void stateChanged(State newState);

    /**
     * Emitted periodically during playback (approx. every 100-500ms).
     * 
     * @param positionMs Current position in milliseconds
     */
    void positionChanged(qint64 positionMs);

    /**
     * Emitted when media duration is determined.
     * 
     * @param durationMs Duration in milliseconds
     */
    void durationChanged(qint64 durationMs);

    /**
     * Emitted when metadata is available or updated.
     * 
     * @param metadata Track metadata
     */
    void metadataChanged(const TrackMetadata& metadata);

    /**
     * Emitted when a playback error occurs.
     * 
     * @param errorMessage Human-readable error message
     */
    void error(const QString& errorMessage);

    /**
     * Emitted when end of media is reached.
     * Used for queue management and gapless playback.
     */
    void endOfStream();

    /**
     * Emitted when buffering progress changes (for streams).
     * 
     * @param percent Buffering percentage (0-100)
     */
    void bufferingChanged(int percent);

  protected:
    IMediaEngine(QObject* parent = nullptr) : QObject(parent) {}
};

}  // namespace media
}  // namespace extensions
}  // namespace opencardev::crankshaft
