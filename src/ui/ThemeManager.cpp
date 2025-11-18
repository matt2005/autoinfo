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

#include "ThemeManager.hpp"
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

namespace CrankshaftReborn {
namespace UI {

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(Theme::defaultLight())
    , m_currentThemeName("light")
{
}

ThemeManager* ThemeManager::instance() {
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

void ThemeManager::initialize(const QString& themesPath) {
    // Set themes path
    if (themesPath.isEmpty()) {
        // Try multiple locations
        QStringList searchPaths = {
            // System install (package)
            "/usr/share/CrankshaftReborn/themes",
            "/usr/share/crankshaft_reborn/themes",
            // Application directory (portable/relocatable)
            QCoreApplication::applicationDirPath() + "/themes",
            // Current working directory (developer runs)
            QDir::currentPath() + "/themes",
            // Per-user data dir
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/themes"
        };
        
        for (const QString& path : searchPaths) {
            if (QDir(path).exists()) {
                m_themesPath = path;
                qDebug() << "Using themes path:" << m_themesPath;
                break;
            }
        }
        
        if (m_themesPath.isEmpty()) {
            qWarning() << "No themes directory found, using built-in themes";
        }
    } else {
        m_themesPath = themesPath;
    }
    
    // Load available themes
    loadAvailableThemes();
    
    // Load saved theme preference or use default
    loadSavedTheme();
}

void ThemeManager::loadAvailableThemes() {
    m_availableThemes.clear();
    
    // Always add built-in themes
    m_availableThemes["light"] = ":builtin:light";
    m_availableThemes["dark"] = ":builtin:dark";
    
    // Load themes from filesystem
    if (!m_themesPath.isEmpty()) {
        QDir themesDir(m_themesPath);
        if (themesDir.exists()) {
            QStringList filters;
            filters << "*.json";
            QFileInfoList files = themesDir.entryInfoList(filters, QDir::Files);
            
            for (const QFileInfo& fileInfo : files) {
                QString themeName = fileInfo.baseName();
                m_availableThemes[themeName] = fileInfo.absoluteFilePath();
                qDebug() << "Found theme:" << themeName << "at" << fileInfo.absoluteFilePath();
            }
        }
    }
}

bool ThemeManager::loadTheme(const QString& themeName) {
    Theme newTheme;
    
    if (!m_availableThemes.contains(themeName)) {
        qWarning() << "Theme not found:" << themeName;
        return false;
    }
    
    QString themePath = m_availableThemes[themeName];
    
    // Handle built-in themes
    if (themePath == ":builtin:light") {
        newTheme = Theme::defaultLight();
    } else if (themePath == ":builtin:dark") {
        newTheme = Theme::defaultDark();
    } else {
        // Load from file
        newTheme = Theme::fromFile(themePath);
    }
    
    // Apply theme
    m_currentTheme = newTheme;
    m_currentThemeName = themeName;
    
    qDebug() << "Loaded theme:" << m_currentTheme.displayName << "(" << themeName << ")";
    
    emit themeChanged();
    saveCurrentTheme();
    
    return true;
}

void ThemeManager::setCurrentTheme(const QString& themeName) {
    if (themeName != m_currentThemeName) {
        loadTheme(themeName);
    }
}

void ThemeManager::toggleTheme() {
    // Toggle between light and dark
    if (m_currentTheme.isDark) {
        setCurrentTheme("light");
    } else {
        setCurrentTheme("dark");
    }
}

QStringList ThemeManager::availableThemes() const {
    return m_availableThemes.keys();
}

void ThemeManager::saveCurrentTheme() {
    QSettings settings("OpenCarDev", "CrankshaftReborn");
    settings.setValue("ui/theme", m_currentThemeName);
    settings.sync();
}

void ThemeManager::loadSavedTheme() {
    QSettings settings("OpenCarDev", "CrankshaftReborn");
    QString savedTheme = settings.value("ui/theme", "light").toString();
    
    if (m_availableThemes.contains(savedTheme)) {
        loadTheme(savedTheme);
    } else {
        // Fallback to light theme
        loadTheme("light");
    }
}

void ThemeManager::registerQmlType() {
    qmlRegisterSingletonType<ThemeManager>(
        "CrankshaftReborn.UI", 1, 0, "ThemeManager",
        [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return ThemeManager::instance();
        }
    );
}

} // namespace UI
} // namespace CrankshaftReborn
