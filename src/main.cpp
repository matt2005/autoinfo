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

#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>
#include "core/application.hpp"
#include "ui/ThemeManager.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("OpenCarDev");
    app.setOrganizationDomain("getcrankshaft.com");
    app.setApplicationName("Crankshaft Reborn");
    app.setApplicationVersion("1.0.0");

    openauto::core::Application application;
    if (!application.initialize()) {
        return 1;
    }

    // Register QML singleton and initialize theme manager
    CrankshaftReborn::UI::ThemeManager::registerQmlType();
    CrankshaftReborn::UI::ThemeManager::instance()->initialize();

    // Set up QML engine and import paths
    QQmlApplicationEngine engine;

    QStringList importPaths;
    // Current working directory (developer runs)
    importPaths << (QDir::currentPath() + "/assets/qml");
    // Application directory (portable/relocatable bundles)
    importPaths << (QCoreApplication::applicationDirPath() + "/qml");
    // System install locations (Debian/Ubuntu packages)
    importPaths << QString::fromUtf8("/usr/share/CrankshaftReborn/qml");
    importPaths << QString::fromUtf8("/usr/share/crankshaft_reborn/qml");
    // Per-user data dir
    importPaths << (QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/qml");
    // Optional override via environment
    const QString envQml = qEnvironmentVariable("CRANKSHAFT_QML_PATH");
    if (!envQml.isEmpty()) {
        importPaths.prepend(envQml);
    }

    for (const QString& p : importPaths) {
        if (QDir(p).exists()) {
            engine.addImportPath(p);
        }
    }

    // Try to find a main QML to load; fall back to ThemeDemo.qml
    QStringList candidates = {"Main.qml", "App.qml", "main.qml", "ThemeDemo.qml"};
    QUrl mainUrl;
    for (const QString& base : importPaths) {
        for (const QString& name : candidates) {
            const QString candidatePath = QDir(base).filePath(name);
            if (QFileInfo::exists(candidatePath)) {
                mainUrl = QUrl::fromLocalFile(candidatePath);
                break;
            }
        }
        if (!mainUrl.isEmpty()) break;
    }

    if (mainUrl.isEmpty()) {
        // As a last resort, attempt to load ThemeDemo.qml from current dir
        const QString fallback = QDir::currentPath() + "/assets/qml/ThemeDemo.qml";
        if (QFileInfo::exists(fallback)) {
            mainUrl = QUrl::fromLocalFile(fallback);
        }
    }

    if (!mainUrl.isEmpty()) {
        engine.load(mainUrl);
    }

    if (engine.rootObjects().isEmpty()) {
        // No UI loaded; keep core services running if desired
        // Return failure for now so CI/dev notices missing UI
        return 1;
    }

    return app.exec();
}
