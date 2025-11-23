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

#include "dialer_extension.hpp"
#include "../../src/core/config/ConfigManager.hpp"
#include "../../src/core/config/ConfigTypes.hpp"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

namespace opencardev::crankshaft {
namespace extensions {
namespace dialer {

bool DialerExtension::initialize() {
    qInfo() << "Initializing Dialler extension...";

    // Event capability (to integrate with Bluetooth later)
    eventCap_ = getCapability<core::capabilities::EventCapability>();
    if (!eventCap_) {
        qWarning() << "Dialler: Event capability not granted; limited functionality.";
    }

    return true;
}

void DialerExtension::start() {
    qInfo() << "Starting Dialler extension...";

    // Register UI main view
    auto uiCap = getCapability<core::capabilities::UICapability>();
    if (uiCap) {
        QVariantMap meta;
        meta["title"] = "Dialler";
        meta["icon"] = "📞"; // simple emoji icon placeholder
        meta["description"] = "Make and manage calls";
        
        // Use file path - extension QML files are copied to build/extensions/{ext}/qml
        QString qmlPath = QDir(QCoreApplication::applicationDirPath()).filePath("extensions/dialer/qml/DialerView.qml");
        if (!QFile::exists(qmlPath)) {
            // Try current working directory
            qmlPath = "extensions/dialer/qml/DialerView.qml";
        }
        qInfo() << "Dialler: Registering view at" << qmlPath;
        uiCap->registerMainView(qmlPath, meta);
    } else {
        qWarning() << "Dialler: UI capability not granted; cannot register view";
    }

    setupEventHandlers();
}

void DialerExtension::stop() {
    qInfo() << "Stopping Dialler extension...";
}

void DialerExtension::cleanup() {
    qInfo() << "Cleaning up Dialler extension...";
    eventCap_.reset();
}

void DialerExtension::registerConfigItems(core::config::ConfigManager* manager) {
    using namespace core::config;

    ConfigPage page;
    page.domain = "phone";
    page.extension = "dialer";
    page.title = "Dialler Settings";
    page.description = "Configure dialler preferences";
    page.icon = "qrc:/icons/phone.svg";

    ConfigSection general;
    general.key = "general";
    general.title = "General";
    general.description = "Dialler preferences";
    general.complexity = ConfigComplexity::Basic;

    ConfigItem lastNum;
    lastNum.key = "last_number";
    lastNum.label = "Last dialled number";
    lastNum.description = "Stores last dialled number";
    lastNum.type = ConfigItemType::String;
    lastNum.defaultValue = QString("");
    lastNum.complexity = ConfigComplexity::Basic;
    lastNum.readOnly = false;
    lastNum.required = false;
    general.items.append(lastNum);

    page.sections.append(general);
    manager->registerConfigPage(page);
}

void DialerExtension::setupEventHandlers() {
    if (!eventCap_) return;

    // Listen for dial requests from other extensions (optional)
    eventCap_->subscribe("*.phone.dial", [this](const QVariantMap& data){
        qInfo() << "Dialler: received external dial request" << data;
    });
}

} // namespace dialer
} // namespace extensions
} // namespace opencardev::crankshaft
