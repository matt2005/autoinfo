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
      capability_manager_(nullptr),
      extension_manager_(std::make_unique<extensions::ExtensionManager>()) {
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    qInfo() << "Initializing Crankshaft Reborn Application (Capability-Based Architecture)...";

    setupEventBus();
    setupWebSocketServer();
    setupCapabilityManager();
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

void Application::setupCapabilityManager() {
    qDebug() << "Setting up capability manager...";
    capability_manager_ = std::make_unique<CapabilityManager>(event_bus_.get(), websocket_server_.get());
    qInfo() << "Capability manager initialized - extensions will use capability-based security";
}

void Application::registerBuiltInExtensions() {
    qDebug() << "Registering built-in extensions...";
    // Built-in extensions will be registered here before loadAll() is called
    // This is called from main.cpp before initialize()
}

void Application::loadExtensions() {
    qDebug() << "Loading extensions with capability-based security...";
    extension_manager_->initialize(capability_manager_.get());
    extension_manager_->loadAll();
}

}  // namespace core
}  // namespace openauto
