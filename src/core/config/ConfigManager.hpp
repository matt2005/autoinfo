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

#ifndef OPENCARDEV_CRANKSHAFT_CORE_CONFIG_CONFIGMANAGER_HPP
#define OPENCARDEV_CRANKSHAFT_CORE_CONFIG_CONFIGMANAGER_HPP

#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include "ConfigTypes.hpp"

namespace opencardev {
namespace crankshaft {
namespace core {
namespace config {

class ConfigManager : public QObject {
    Q_OBJECT

  public:
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager() override;

    // Configuration registration
    void registerConfigPage(const ConfigPage& page);
    void unregisterConfigPage(const QString& domain, const QString& extension);

    // Page access
    bool hasConfigPage(const QString& domain, const QString& extension) const;
    ConfigPage getConfigPage(const QString& domain, const QString& extension) const;
    QList<ConfigPage> getAllConfigPages() const;
    QList<ConfigPage> getConfigPagesByDomain(const QString& domain) const;

    // Value access (domain.extension.section.key)
    QVariant getValue(const QString& domain, const QString& extension, const QString& section,
                      const QString& key) const;
    QVariant getValue(
        const QString& fullPath) const;  // e.g., "core.wireless.connection.autoconnect"

    bool setValue(const QString& domain, const QString& extension, const QString& section,
                  const QString& key, const QVariant& value);
    bool setValue(const QString& fullPath, const QVariant& value);

    // Reset to defaults
    void resetToDefaults(const QString& domain, const QString& extension);
    void resetSectionToDefaults(const QString& domain, const QString& extension,
                                const QString& section);
    void resetItemToDefault(const QString& domain, const QString& extension, const QString& section,
                            const QString& key);

    // Persistence
    bool save();
    bool load();
    bool saveExtensionConfig(const QString& domain, const QString& extension);
    bool loadExtensionConfig(const QString& domain, const QString& extension);

    // Export/Import
    QVariantMap exportConfig(bool maskSecrets = true) const;
    QVariantMap exportConfig(const QStringList& domainExtensions, bool maskSecrets = true) const;
    bool importConfig(const QVariantMap& config, bool overwriteExisting = false);
    bool importConfig(const QVariantMap& config, const QStringList& domainExtensions,
                      bool overwriteExisting = false);

    // Backup/Restore
    bool backupToFile(const QString& filePath, bool maskSecrets = false, bool compress = true);
    bool backupToFile(const QString& filePath, const QStringList& domainExtensions,
                      bool maskSecrets = false, bool compress = true);
    bool restoreFromFile(const QString& filePath, bool overwriteExisting = false);
    bool restoreFromFile(const QString& filePath, const QStringList& domainExtensions,
                         bool overwriteExisting = false);

    // Migration helpers
    bool applyConfigChanges(const QVariantMap& config, bool overwriteExisting = false);

    // Complexity level management
    void setComplexityLevel(ConfigComplexity level);
    ConfigComplexity getComplexityLevel() const;

  signals:
    void configValueChanged(const QString& domain, const QString& extension, const QString& section,
                            const QString& key, const QVariant& value);
    void configPageRegistered(const QString& domain, const QString& extension);
    void configPageUnregistered(const QString& domain, const QString& extension);
    void complexityLevelChanged(ConfigComplexity level);

  private:
    QString getConfigFilePath(const QString& domain, const QString& extension) const;
    QString makeKey(const QString& domain, const QString& extension) const;
    bool parseFullPath(const QString& fullPath, QString& domain, QString& extension,
                       QString& section, QString& key) const;

    QVariantMap exportConfigPage(const ConfigPage& page, bool maskSecrets) const;
    bool importConfigPage(const QString& domain, const QString& extension,
                          const QVariantMap& pageData, bool overwriteExisting);
    bool compressToFile(const QByteArray& data, const QString& filePath);
    QByteArray decompressFromFile(const QString& filePath, bool& success);

    QMap<QString, ConfigPage> config_pages_;  // Key: "domain.extension"
    ConfigComplexity current_complexity_;
};

}  // namespace config
}  // namespace core
}  // namespace crankshaft
}  // namespace opencardev

#endif  // OPENCARDEV_CRANKSHAFT_CORE_CONFIG_CONFIGMANAGER_HPP
