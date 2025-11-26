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

#include "application.hpp"
#include <QDebug>
#include "../../extensions/extension_manager.hpp"
#include "../config/ConfigManager.hpp"

namespace opencardev::crankshaft::core {

Application::Application(QObject* parent)
    : QObject(parent),
      event_bus_(std::make_unique<EventBus>()),
      websocket_server_(std::make_unique<WebSocketServer>()),
      capability_manager_(nullptr),
      config_manager_(nullptr),
      extension_manager_(nullptr) {}

Application::~Application() {
    shutdown();
    delete config_manager_;
    delete extension_manager_;
}

auto Application::initialize() -> bool {
    qInfo() << "Initializing Crankshaft Reborn Application (Capability-Based Architecture)...";

    setupEventBus();
    setupWebSocketServer();
    setupCapabilityManager();
    setupConfigManager();
    loadExtensions();

    qInfo() << "Application initialized successfully";
    return true;
}

void Application::shutdown() {
    qInfo() << "Shutting down application...";

    if (extension_manager_ != nullptr) {
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
    constexpr int kDefaultWebsocketPort = 8080;
    websocket_server_->start(kDefaultWebsocketPort);
}

void Application::setupCapabilityManager() {
    qDebug() << "Setting up capability manager...";
    capability_manager_ =
        std::make_unique<CapabilityManager>(event_bus_.get(), websocket_server_.get());
    qInfo() << "Capability manager initialized - extensions will use capability-based security";
}

void Application::setupConfigManager() {
    qDebug() << "Setting up config manager...";
    config_manager_ = new opencardev::crankshaft::core::config::ConfigManager();
    config_manager_->load();
    qInfo() << "Config manager initialized";
}

auto Application::configManager() const -> opencardev::crankshaft::core::config::ConfigManager* {
    return config_manager_;
}

auto Application::extensionManager() const -> opencardev::crankshaft::extensions::ExtensionManager* {
    return extension_manager_;
}

void Application::registerBuiltInExtensions() {
    qDebug() << "Registering built-in extensions...";
    // Create extension manager if not already created
    if (extension_manager_ == nullptr) {
        extension_manager_ = new opencardev::crankshaft::extensions::ExtensionManager();
    }
    // Built-in extensions will be registered here before loadAll() is called
    // This is called from main.cpp before initialize()
}

void Application::loadExtensions() {
    qDebug() << "Loading extensions with capability-based security...";
    // Create extension manager if not already created
    if (extension_manager_ == nullptr) {
        extension_manager_ = new opencardev::crankshaft::extensions::ExtensionManager();
    }
    extension_manager_->initialize(capability_manager_.get(), config_manager_);
    extension_manager_->loadAll();
}

}  // namespace opencardev::crankshaft::core
