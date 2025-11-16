/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../../src/extensions/extension.hpp"

namespace openauto {
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

private:
    void setupEventHandlers();
    void handlePlayCommand(const QVariantMap& data);
    void handlePauseCommand(const QVariantMap& data);
    void handleStopCommand(const QVariantMap& data);
    void handleNextCommand(const QVariantMap& data);
    void handlePreviousCommand(const QVariantMap& data);
};

}  // namespace media
}  // namespace extensions
}  // namespace openauto
