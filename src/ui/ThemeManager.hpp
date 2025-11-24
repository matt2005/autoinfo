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

#include <QMap>
#include <QObject>
#include <QQmlEngine>
#include <memory>
#include "Theme.hpp"

namespace CrankshaftReborn {
namespace UI {

/**
 * @brief Singleton theme manager for application-wide theme control
 */
class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY themeChanged)
    Q_PROPERTY(bool isDark READ isDark NOTIFY themeChanged)

    // Expose theme colors as properties for QML
    Q_PROPERTY(QColor primaryColor READ primaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor primaryLightColor READ primaryLightColor NOTIFY themeChanged)
    Q_PROPERTY(QColor primaryDarkColor READ primaryDarkColor NOTIFY themeChanged)
    Q_PROPERTY(QColor secondaryColor READ secondaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor surfaceColor READ surfaceColor NOTIFY themeChanged)
    Q_PROPERTY(QColor cardColor READ cardColor NOTIFY themeChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY themeChanged)
    Q_PROPERTY(QColor textSecondaryColor READ textSecondaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor dividerColor READ dividerColor NOTIFY themeChanged)
    Q_PROPERTY(QColor borderColor READ borderColor NOTIFY themeChanged)
    Q_PROPERTY(QColor successColor READ successColor NOTIFY themeChanged)
    Q_PROPERTY(QColor warningColor READ warningColor NOTIFY themeChanged)
    Q_PROPERTY(QColor errorColor READ errorColor NOTIFY themeChanged)
    Q_PROPERTY(QColor infoColor READ infoColor NOTIFY themeChanged)

    Q_PROPERTY(int cornerRadius READ cornerRadius NOTIFY themeChanged)
    Q_PROPERTY(int spacing READ spacing NOTIFY themeChanged)
    Q_PROPERTY(int padding READ padding NOTIFY themeChanged)
    Q_PROPERTY(QStringList availableThemes READ availableThemes CONSTANT)

  public:
    /**
     * @brief Get singleton instance
     */
    static ThemeManager* instance();

    /**
     * @brief Initialize theme system
     * @param themesPath Path to themes directory
     */
    void initialize(const QString& themesPath = QString());

    /**
     * @brief Load theme from file
     * @param themeName Theme identifier
     * @return True if loaded successfully
     */
    bool loadTheme(const QString& themeName);

    /**
     * @brief Get list of available theme names
     */
    QStringList availableThemes() const;

    /**
     * @brief Register theme manager as QML singleton
     */
    static void registerQmlType();

    // Property getters
    QString currentTheme() const { return m_currentThemeName; }
    bool isDark() const { return m_currentTheme.isDark; }

    QColor primaryColor() const { return m_currentTheme.primaryColor; }
    QColor primaryLightColor() const { return m_currentTheme.primaryLightColor; }
    QColor primaryDarkColor() const { return m_currentTheme.primaryDarkColor; }
    QColor secondaryColor() const { return m_currentTheme.secondaryColor; }
    QColor accentColor() const { return m_currentTheme.accentColor; }
    QColor backgroundColor() const { return m_currentTheme.backgroundColor; }
    QColor surfaceColor() const { return m_currentTheme.surfaceColor; }
    QColor cardColor() const { return m_currentTheme.cardColor; }
    QColor textColor() const { return m_currentTheme.textColor; }
    QColor textSecondaryColor() const { return m_currentTheme.textSecondaryColor; }
    QColor dividerColor() const { return m_currentTheme.dividerColor; }
    QColor borderColor() const { return m_currentTheme.borderColor; }
    QColor successColor() const { return m_currentTheme.successColor; }
    QColor warningColor() const { return m_currentTheme.warningColor; }
    QColor errorColor() const { return m_currentTheme.errorColor; }
    QColor infoColor() const { return m_currentTheme.infoColor; }

    int cornerRadius() const { return m_currentTheme.cornerRadius; }
    int spacing() const { return m_currentTheme.spacing; }
    int padding() const { return m_currentTheme.padding; }

  public slots:
    void setCurrentTheme(const QString& themeName);
    void toggleTheme();

  signals:
    void themeChanged();

  private:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() override = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void loadAvailableThemes();
    void saveCurrentTheme();
    void loadSavedTheme();

    static ThemeManager* s_instance;

    Theme m_currentTheme;
    QString m_currentThemeName;
    QString m_themesPath;
    QMap<QString, QString> m_availableThemes;  // name -> file path
};

}  // namespace UI
}  // namespace CrankshaftReborn
