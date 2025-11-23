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
#include "ui/I18nManager.hpp"
#include "ui/NavigationBridge.hpp"
#include "ui/EventBridge.hpp"
#include "ui/IconRegistry.hpp"
// Config UI bridge
#include "ui/ConfigManagerBridge.hpp"
// Registrar decoupling: inject UI registrar into core
#include "core/ui/UIRegistrar.hpp"
#include "ui/UIRegistrarImpl.hpp"
// Temporarily disabled due to GCC 14/Qt6 ABI incompatibility
// #include "ui/BluetoothBridge.hpp"
#include "../extensions/navigation/navigation_extension.hpp"
#include "../extensions/bluetooth/bluetooth_extension.hpp"
#include "../extensions/media_player/media_player_extension.hpp"
#include "../extensions/dialer/dialer_extension.hpp"
#include "../extensions/wireless/wireless_extension.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("OpenCarDev");
    app.setOrganizationDomain("getcrankshaft.com");
    app.setApplicationName("Crankshaft Reborn");
    app.setApplicationVersion("1.0.0");

    opencardev::crankshaft::core::Application application;
    
    // Register built-in extensions BEFORE initialize()
    // Navigation extension
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
    
    // Bluetooth extension
    auto bluetoothExtension = std::make_shared<opencardev::crankshaft::extensions::bluetooth::BluetoothExtension>();
    QString btExtensionPath = QDir(QCoreApplication::applicationDirPath()).filePath("extensions/bluetooth");
    if (!QFile::exists(btExtensionPath + "/manifest.json")) {
        btExtensionPath = QDir::currentPath() + "/build/extensions/bluetooth";
        if (!QFile::exists(btExtensionPath + "/manifest.json")) {
            btExtensionPath = "../extensions/bluetooth";
        }
    }
    
    // Media Player extension
    auto mediaPlayerExtension = std::make_shared<opencardev::crankshaft::extensions::media::MediaPlayerExtension>();
    QString mpExtensionPath = QDir(QCoreApplication::applicationDirPath()).filePath("extensions/media_player");
    if (!QFile::exists(mpExtensionPath + "/manifest.json")) {
        mpExtensionPath = QDir::currentPath() + "/build/extensions/media_player";
        if (!QFile::exists(mpExtensionPath + "/manifest.json")) {
            mpExtensionPath = "../extensions/media_player";
        }
    }
    
    // Dialer extension
    auto dialerExtension = std::make_shared<opencardev::crankshaft::extensions::dialer::DialerExtension>();
    QString dialerExtensionPath = QDir(QCoreApplication::applicationDirPath()).filePath("extensions/dialer");
    if (!QFile::exists(dialerExtensionPath + "/manifest.json")) {
        dialerExtensionPath = QDir::currentPath() + "/build/extensions/dialer";
        if (!QFile::exists(dialerExtensionPath + "/manifest.json")) {
            dialerExtensionPath = "../extensions/dialer";
        }
    }
    
    // Wireless extension
    auto wirelessExtension = std::make_shared<opencardev::crankshaft::extensions::wireless::WirelessExtension>();
    QString wirelessExtensionPath = QDir(QCoreApplication::applicationDirPath()).filePath("extensions/wireless");
    if (!QFile::exists(wirelessExtensionPath + "/manifest.json")) {
        wirelessExtensionPath = QDir::currentPath() + "/build/extensions/wireless";
        if (!QFile::exists(wirelessExtensionPath + "/manifest.json")) {
            wirelessExtensionPath = "../extensions/wireless";
        }
    }
    
    if (!application.initialize()) {
        return 1;
    }
    
    // Initialize icon resources from static library
    Q_INIT_RESOURCE(icons);
    
    // Register QML singletons and initialize managers
    CrankshaftReborn::UI::ThemeManager::registerQmlType();
    CrankshaftReborn::UI::ThemeManager::instance()->initialize();
    // I18n manager for translations
    opencardev::crankshaft::ui::I18nManager::registerQmlType();
    // Icon registry singleton
    opencardev::crankshaft::ui::IconRegistry::registerQmlType();
    NavigationBridge::registerQmlType();
    NavigationBridge::initialise(application.capabilityManager());
    // QML Event bridge for simple publish from UI
    opencardev::crankshaft::ui::EventBridge::registerQmlType();
    opencardev::crankshaft::ui::EventBridge::initialise(application.eventBus());
    // Config Manager bridge for QML Config UI
    opencardev::crankshaft::ui::ConfigManagerBridge::registerQmlType();
    opencardev::crankshaft::ui::ConfigManagerBridge::initialise(application.configManager());
    // Temporarily disabled due to GCC 14/Qt6 ABI incompatibility
    // BluetoothBridge::registerQmlType();
    // BluetoothBridge::initialise(&application);
    
    // Create ExtensionRegistry BEFORE starting extensions so they can register views
    opencardev::crankshaft::ui::ExtensionRegistry extensionRegistry(application.extensionManager());
    opencardev::crankshaft::ui::ExtensionRegistry::registerQmlType();
    // Wire signal/slot for UI component cleanup when extensions disabled
    QObject::connect(application.extensionManager(), 
                     &opencardev::crankshaft::extensions::ExtensionManager::requestUnregisterComponents,
                     &extensionRegistry,
                     &opencardev::crankshaft::ui::ExtensionRegistry::unregisterExtensionComponents);

    // Register core UI settings page
    {
        using namespace opencardev::crankshaft::core::config;
        ConfigPage page;
        page.domain = "system";
        page.extension = "ui";
        page.title = QObject::tr("User Interface");
        page.description = QObject::tr("Global UI preferences including keyboard shortcuts");
        page.icon = "Settings"; // icon name or path; translation not required
        page.complexity = ConfigComplexity::Basic;

        // General section
        ConfigSection general;
        general.key = "general";
        general.title = QObject::tr("General");
        general.description = QObject::tr("General user interface preferences");
        general.complexity = ConfigComplexity::Basic;

        ConfigItem language;
        language.key = "language";
        language.label = QObject::tr("Language");
        language.description = QObject::tr("Application language (requires translation files)");
        language.type = ConfigItemType::Selection;
        language.properties["options"] = QStringList{"en_GB"};
        language.defaultValue = QStringLiteral("en_GB");
        language.complexity = ConfigComplexity::Basic;
        general.items.append(language);

        ConfigSection shortcuts;
        shortcuts.key = "shortcuts";
        shortcuts.title = "Keyboard Shortcuts";
        shortcuts.description = "Configure global shortcut keys";
        shortcuts.complexity = ConfigComplexity::Basic;

        ConfigItem openSettings;
        openSettings.key = "open_settings";
        openSettings.label = "Open settings";
        openSettings.description = "Shortcut key to open the Settings page";
        openSettings.type = ConfigItemType::String;
        openSettings.defaultValue = QStringLiteral("S");
        openSettings.complexity = ConfigComplexity::Basic;
        shortcuts.items.append(openSettings);

        ConfigItem toggleTheme;
        toggleTheme.key = "toggle_theme";
        toggleTheme.label = "Toggle theme";
        toggleTheme.description = "Shortcut key to toggle light/dark theme";
        toggleTheme.type = ConfigItemType::String;
        toggleTheme.defaultValue = QStringLiteral("T");
        toggleTheme.complexity = ConfigComplexity::Basic;
        shortcuts.items.append(toggleTheme);

        ConfigItem goHome;
        goHome.key = "go_home";
        goHome.label = "Go to Home";
        goHome.description = "Shortcut key to switch to the Home tab";
        goHome.type = ConfigItemType::String;
        goHome.defaultValue = QStringLiteral("H");
        goHome.complexity = ConfigComplexity::Basic;
        shortcuts.items.append(goHome);

        ConfigItem cycleLeft;
        cycleLeft.key = "cycle_left";
        cycleLeft.label = "Cycle tabs left";
        cycleLeft.description = "Shortcut key to cycle to the previous tab";
        cycleLeft.type = ConfigItemType::String;
        cycleLeft.defaultValue = QStringLiteral("A");
        cycleLeft.complexity = ConfigComplexity::Basic;
        shortcuts.items.append(cycleLeft);

        ConfigItem cycleRight;
        cycleRight.key = "cycle_right";
        cycleRight.label = "Cycle tabs right";
        cycleRight.description = "Shortcut key to cycle to the next tab";
        cycleRight.type = ConfigItemType::String;
        cycleRight.defaultValue = QStringLiteral("D");
        cycleRight.complexity = ConfigComplexity::Basic;
        shortcuts.items.append(cycleRight);

        ConfigItem showHelp;
        showHelp.key = "show_help";
        showHelp.label = "Show shortcuts help";
        showHelp.description = "Shortcut key to toggle the on-screen shortcuts help overlay";
        showHelp.type = ConfigItemType::String;
        showHelp.defaultValue = QStringLiteral("?");
        showHelp.complexity = ConfigComplexity::Basic;
        shortcuts.items.append(showHelp);

        page.sections.append(general);
        page.sections.append(shortcuts);
        application.configManager()->registerConfigPage(page);
    }

    // Register system.extensions management page (enable/disable + hot reload)
    {
        using namespace opencardev::crankshaft::core::config;
        ConfigPage page;
        page.domain = "system";
        page.extension = "extensions";
        page.title = "Extensions";
        page.description = "Enable or disable built-in extensions";
        page.icon = "Extensions";
        page.complexity = ConfigComplexity::Basic;

        ConfigSection manage;
        manage.key = "manage";
        manage.title = "Manage Extensions";
        manage.description = "Toggle extensions on or off";
        manage.complexity = ConfigComplexity::Basic;

        auto makeToggle = [](const QString& key, const QString& label, const QString& desc) {
            ConfigItem item;
            item.key = key;
            item.label = label;
            item.description = desc;
            item.type = ConfigItemType::Boolean;
            item.defaultValue = true;
            item.complexity = ConfigComplexity::Basic;
            return item;
        };

        manage.items.append(makeToggle("navigation", "Enable Navigation", "Show the Navigation tab and services"));
        manage.items.append(makeToggle("bluetooth", "Enable Bluetooth", "Enable Bluetooth integration"));
        manage.items.append(makeToggle("media_player", "Enable Media Player", "Enable media playback controls"));
        manage.items.append(makeToggle("dialer", "Enable Dialler", "Enable phone dialler integration"));
        manage.items.append(makeToggle("wireless", "Enable Wireless", "Enable wireless settings integration"));

        page.sections.append(manage);
        application.configManager()->registerConfigPage(page);
    }

    // Inject UI registrar implementation into core (decouples core from UI)
    opencardev::crankshaft::ui::UIRegistrarImpl uiRegistrar;
    application.capabilityManager()->setUIRegistrar(&uiRegistrar);
    
    // Now register the built-in extension (after ExtensionRegistry is created)
    application.extensionManager()->registerBuiltInExtension(navigationExtension, navExtensionPath);
    application.extensionManager()->registerBuiltInExtension(bluetoothExtension, btExtensionPath);
    application.extensionManager()->registerBuiltInExtension(mediaPlayerExtension, mpExtensionPath);
    application.extensionManager()->registerBuiltInExtension(dialerExtension, dialerExtensionPath);
    application.extensionManager()->registerBuiltInExtension(wirelessExtension, wirelessExtensionPath);

    // Set up QML engine and import paths
    QQmlApplicationEngine engine;
    // Initialise i18n with engine and ext manager
    opencardev::crankshaft::ui::I18nManager::initialise(&engine, application.extensionManager());
    // Apply language from config (default en_GB)
    {
        const QVariant lang = application.configManager()->getValue("system", "ui", "general", "language");
        const QString locale = lang.isValid() ? lang.toString() : QStringLiteral("en_GB");
        opencardev::crankshaft::ui::I18nManager::instance()->setLocale(locale);
    }
    
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
