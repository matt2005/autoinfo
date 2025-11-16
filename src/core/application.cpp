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

#include "application.hpp"
#include <QDebug>

namespace openauto {
namespace core {

Application::Application(QObject *parent)
    : QObject(parent),
      event_bus_(std::make_unique<EventBus>()),
      websocket_server_(std::make_unique<WebSocketServer>()),
      extension_manager_(std::make_unique<extensions::ExtensionManager>()) {
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    qInfo() << "Initializing Crankshaft Reborn Application...";

    setupEventBus();
    setupWebSocketServer();
    loadExtensions();

    qInfo() << "Application initialized successfully";
    return true;
}

void Application::shutdown() {
    qInfo() << "Shutting down application...";
    
    if (extension_manager_) {
        extension_manager_->unloadAll();
    }
    
    if (websocket_server_) {
        websocket_server_->stop();
    }
}

void Application::setupEventBus() {
    qDebug() << "Setting up event bus...";
    // Event bus initialization
}

void Application::setupWebSocketServer() {
    qDebug() << "Setting up WebSocket server...";
    websocket_server_->start(8080);
}

void Application::loadExtensions() {
    qDebug() << "Loading extensions...";
    extension_manager_->initialize(event_bus_.get(), websocket_server_.get());
    extension_manager_->loadAll();
}

}  // namespace core
}  // namespace openauto
