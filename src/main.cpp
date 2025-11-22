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

#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>
#include "core/application/application.hpp"
#include "extensions/extension_manager.hpp"
#include "ui/ThemeManager.hpp"
#include "ui/ExtensionRegistry.hpp"
#include "ui/NavigationBridge.hpp"
// Registrar decoupling: inject UI registrar into core
#include "core/ui/UIRegistrar.hpp"
#include "ui/UIRegistrarImpl.hpp"
// Temporarily disabled due to GCC 14/Qt6 ABI incompatibility
// #include "ui/BluetoothBridge.hpp"
#include "../extensions/navigation/navigation_extension.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("OpenCarDev");
    app.setOrganizationDomain("getcrankshaft.com");
    app.setApplicationName("Crankshaft Reborn");
    app.setApplicationVersion("1.0.0");

    opencardev::crankshaft::core::Application application;
    
    // Register built-in extensions BEFORE initialize()
    auto navigationExtension = std::make_shared<opencardev::crankshaft::extensions::navigation::NavigationExtension>();
    QString navExtensionPath = QDir(QCoreApplication::applicationDirPath()).filePath("extensions/navigation");
    if (!QFile::exists(navExtensionPath + "/manifest.json")) {
        // Try build directory path
        navExtensionPath = QDir::currentPath() + "/build/extensions/navigation";
        if (!QFile::exists(navExtensionPath + "/manifest.json")) {
            // Try relative to executable
            navExtensionPath = "../extensions/navigation";
        }
    }
    
    if (!application.initialize()) {
        return 1;
    }
    
    // Register QML singletons and initialize managers
    CrankshaftReborn::UI::ThemeManager::registerQmlType();
    CrankshaftReborn::UI::ThemeManager::instance()->initialize();
    NavigationBridge::registerQmlType();
    NavigationBridge::initialise(application.capabilityManager());
    // Temporarily disabled due to GCC 14/Qt6 ABI incompatibility
    // BluetoothBridge::registerQmlType();
    // BluetoothBridge::initialise(&application);
    
    // Create ExtensionRegistry BEFORE starting extensions so they can register views
    opencardev::crankshaft::ui::ExtensionRegistry extensionRegistry(application.extensionManager());
    opencardev::crankshaft::ui::ExtensionRegistry::registerQmlType();

    // Inject UI registrar implementation into core (decouples core from UI)
    opencardev::crankshaft::ui::UIRegistrarImpl uiRegistrar;
    application.capabilityManager()->setUIRegistrar(&uiRegistrar);
    
    // Now register the built-in extension (after ExtensionRegistry is created)
    application.extensionManager()->registerBuiltInExtension(navigationExtension, navExtensionPath);

    // Set up QML engine and import paths
    QQmlApplicationEngine engine;
    
    // Expose ThemeManager as a context property so it's available in all QML files
    engine.rootContext()->setContextProperty("ThemeManager", CrankshaftReborn::UI::ThemeManager::instance());
    engine.rootContext()->setContextProperty("NavigationBridge", NavigationBridge::instance());
    // Temporarily disabled due to GCC 14/Qt6 ABI incompatibility
    // engine.rootContext()->setContextProperty("BluetoothBridge", BluetoothBridge::instance());

    QStringList importPaths;
    // Qt6 system QML modules (for QtPositioning, QtLocation, etc.)
    importPaths << QString::fromUtf8("/usr/lib/x86_64-linux-gnu/qt6/qml");
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
    qDebug() << "Searching for QML file in import paths:";
    for (const QString& base : importPaths) {
        qDebug() << "  Checking path:" << base;
        for (const QString& name : candidates) {
            const QString candidatePath = QDir(base).filePath(name);
            if (QFileInfo::exists(candidatePath)) {
                mainUrl = QUrl::fromLocalFile(candidatePath);
                qDebug() << "  Found QML file:" << candidatePath;
                break;
            }
        }
        if (!mainUrl.isEmpty()) break;
    }

    if (mainUrl.isEmpty()) {
        // As a last resort, attempt to load ThemeDemo.qml from current dir
        const QString fallback = QDir::currentPath() + "/assets/qml/ThemeDemo.qml";
        qDebug() << "Trying fallback:" << fallback;
        if (QFileInfo::exists(fallback)) {
            mainUrl = QUrl::fromLocalFile(fallback);
            qDebug() << "Fallback found!";
        }
    }

    if (!mainUrl.isEmpty()) {
        qDebug() << "Loading QML from:" << mainUrl;
        engine.load(mainUrl);
    } else {
        qWarning() << "No QML file found to load!";
    }

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "No root objects after loading QML - UI failed to load";
        // No UI loaded; keep core services running if desired
        // Return failure for now so CI/dev notices missing UI
        return 1;
    }
    
    qDebug() << "QML loaded successfully, entering event loop";

    return app.exec();
}
