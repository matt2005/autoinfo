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

#include <QString>
#include <QVariantMap>
#include <memory>

namespace openauto {
namespace core {
    class EventBus;
    class WebSocketServer;
}

namespace extensions {

enum class ExtensionType {
    Unknown,
    Service,      // Background services (e.g., Bluetooth, GPS)
    UI,           // UI components (e.g., Media player UI)
    Integration,  // Third-party integrations
    Platform      // Platform-specific features
};

class Extension {
public:
    virtual ~Extension() = default;

    // Lifecycle methods
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    // Metadata
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual ExtensionType type() const = 0;

    // Dependencies
    void setEventBus(core::EventBus* event_bus) { event_bus_ = event_bus; }
    void setWebSocketServer(core::WebSocketServer* ws_server) { ws_server_ = ws_server; }

protected:
    core::EventBus* event_bus_ = nullptr;
    core::WebSocketServer* ws_server_ = nullptr;
};

}  // namespace extensions
}  // namespace openauto
