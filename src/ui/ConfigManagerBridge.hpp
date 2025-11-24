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

#include <qqml.h>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

namespace opencardev::crankshaft::core::config {
class ConfigManager;
}

namespace opencardev::crankshaft::ui {

class ConfigManagerBridge : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

  public:
    static ConfigManagerBridge* instance();
    static void registerQmlType();
    static void initialise(core::config::ConfigManager* manager);

    // Config pages
    Q_INVOKABLE QVariantList getAllConfigPages() const;
    Q_INVOKABLE QVariantList getConfigPagesByDomain(const QString& domain) const;
    Q_INVOKABLE QVariantMap getConfigPage(const QString& domain, const QString& extension) const;

    // Value access
    Q_INVOKABLE QVariant getValue(const QString& domain, const QString& extension,
                                  const QString& section, const QString& key) const;
    Q_INVOKABLE QVariant getValue(const QString& fullPath) const;
    Q_INVOKABLE bool setValue(const QString& domain, const QString& extension,
                              const QString& section, const QString& key, const QVariant& value);
    Q_INVOKABLE bool setValue(const QString& fullPath, const QVariant& value);

    // Reset operations
    Q_INVOKABLE void resetToDefaults(const QString& domain, const QString& extension);
    Q_INVOKABLE void resetSectionToDefaults(const QString& domain, const QString& extension,
                                            const QString& section);
    Q_INVOKABLE void resetItemToDefault(const QString& domain, const QString& extension,
                                        const QString& section, const QString& key);

    // Save/Load
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool load();

    // Export/Import
    Q_INVOKABLE QVariantMap exportConfig(bool maskSecrets = true) const;
    Q_INVOKABLE bool importConfig(const QVariantMap& config, bool overwriteExisting = false);

    // Backup/Restore
    Q_INVOKABLE bool backupToFile(const QString& filePath, bool maskSecrets = false,
                                  bool compress = true);
    Q_INVOKABLE bool backupToFile(const QString& filePath, const QStringList& domainExtensions,
                                  bool maskSecrets = false, bool compress = true);
    Q_INVOKABLE bool restoreFromFile(const QString& filePath, bool overwriteExisting = false);
    Q_INVOKABLE bool restoreFromFile(const QString& filePath, const QStringList& domainExtensions,
                                     bool overwriteExisting = false);

    // Complexity level
    Q_INVOKABLE QString getComplexityLevel() const;
    Q_INVOKABLE void setComplexityLevel(const QString& level);
    Q_INVOKABLE QStringList getComplexityLevels() const;

  signals:
    void configValueChanged(const QString& domain, const QString& extension, const QString& section,
                            const QString& key, const QVariant& value);
    void configPageRegistered(const QString& domain, const QString& extension);
    void complexityLevelChanged(const QString& level);

  private:
    explicit ConfigManagerBridge(QObject* parent = nullptr);
    ~ConfigManagerBridge() override = default;

    void connectSignals();

    static ConfigManagerBridge* instance_;
    core::config::ConfigManager* config_manager_;
};

}  // namespace opencardev::crankshaft::ui
