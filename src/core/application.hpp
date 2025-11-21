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
#include <QObject>
#include "event_bus.hpp"
#include "websocket_server.hpp"
#include "CapabilityManager.hpp"
#include "../extensions/extension_manager.hpp"

namespace openauto {
namespace core {

class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    bool initialize();
    void registerBuiltInExtensions();
    void shutdown();

    EventBus* eventBus() const { return event_bus_.get(); }
    WebSocketServer* webSocketServer() const { return websocket_server_.get(); }
    CapabilityManager* capabilityManager() const { return capability_manager_.get(); }
    extensions::ExtensionManager* extensionManager() const { return extension_manager_.get(); }

private:
    void setupEventBus();
    void setupWebSocketServer();
    void setupCapabilityManager();
    void loadExtensions();

    std::unique_ptr<EventBus> event_bus_;
    std::unique_ptr<WebSocketServer> websocket_server_;
    std::unique_ptr<CapabilityManager> capability_manager_;
    std::unique_ptr<extensions::ExtensionManager> extension_manager_;
};

}  // namespace core
}  // namespace openauto
