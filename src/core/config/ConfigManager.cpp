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

#include "ConfigManager.hpp"
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace opencardev {
namespace crankshaft {
namespace core {
namespace config {

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent), current_complexity_(ConfigComplexity::Basic) {}

ConfigManager::~ConfigManager() {
    save();
}

void ConfigManager::registerConfigPage(const ConfigPage& page) {
    QString key = makeKey(page.domain, page.extension);
    config_pages_[key] = page;

    // Load saved values if they exist
    loadExtensionConfig(page.domain, page.extension);

    qInfo() << "Registered config page:" << key;
    emit configPageRegistered(page.domain, page.extension);
}

void ConfigManager::unregisterConfigPage(const QString& domain, const QString& extension) {
    QString key = makeKey(domain, extension);
    if (config_pages_.remove(key)) {
        qInfo() << "Unregistered config page:" << key;
        emit configPageUnregistered(domain, extension);
    }
}

bool ConfigManager::hasConfigPage(const QString& domain, const QString& extension) const {
    return config_pages_.contains(makeKey(domain, extension));
}

ConfigPage ConfigManager::getConfigPage(const QString& domain, const QString& extension) const {
    return config_pages_.value(makeKey(domain, extension));
}

QList<ConfigPage> ConfigManager::getAllConfigPages() const {
    return config_pages_.values();
}

QList<ConfigPage> ConfigManager::getConfigPagesByDomain(const QString& domain) const {
    QList<ConfigPage> result;
    for (const ConfigPage& page : config_pages_.values()) {
        if (page.domain == domain) {
            result.append(page);
        }
    }
    return result;
}

QVariant ConfigManager::getValue(const QString& domain, const QString& extension,
                                 const QString& section, const QString& key) const {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        return QVariant();
    }

    const ConfigPage& page = config_pages_[pageKey];
    for (const ConfigSection& sec : page.sections) {
        if (sec.key == section) {
            for (const ConfigItem& item : sec.items) {
                if (item.key == key) {
                    return item.currentValue.isValid() ? item.currentValue : item.defaultValue;
                }
            }
        }
    }

    return QVariant();
}

QVariant ConfigManager::getValue(const QString& fullPath) const {
    QString domain, extension, section, key;
    if (!parseFullPath(fullPath, domain, extension, section, key)) {
        return QVariant();
    }
    return getValue(domain, extension, section, key);
}

bool ConfigManager::setValue(const QString& domain, const QString& extension,
                             const QString& section, const QString& key, const QVariant& value) {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        return false;
    }

    ConfigPage& page = config_pages_[pageKey];
    for (ConfigSection& sec : page.sections) {
        if (sec.key == section) {
            for (ConfigItem& item : sec.items) {
                if (item.key == key) {
                    if (item.readOnly) {
                        qWarning() << "Attempt to set read-only config item:" << domain << "."
                                   << extension << "." << section << "." << key;
                        return false;
                    }
                    item.currentValue = value;
                    saveExtensionConfig(domain, extension);
                    emit configValueChanged(domain, extension, section, key, value);
                    return true;
                }
            }
        }
    }

    return false;
}

bool ConfigManager::setValue(const QString& fullPath, const QVariant& value) {
    QString domain, extension, section, key;
    if (!parseFullPath(fullPath, domain, extension, section, key)) {
        return false;
    }
    return setValue(domain, extension, section, key, value);
}

void ConfigManager::resetToDefaults(const QString& domain, const QString& extension) {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        return;
    }

    ConfigPage& page = config_pages_[pageKey];
    for (ConfigSection& section : page.sections) {
        for (ConfigItem& item : section.items) {
            item.currentValue = item.defaultValue;
            emit configValueChanged(domain, extension, section.key, item.key, item.defaultValue);
        }
    }

    saveExtensionConfig(domain, extension);
    qInfo() << "Reset config to defaults:" << pageKey;
}

void ConfigManager::resetSectionToDefaults(const QString& domain, const QString& extension,
                                           const QString& sectionKey) {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        return;
    }

    ConfigPage& page = config_pages_[pageKey];
    for (ConfigSection& section : page.sections) {
        if (section.key == sectionKey) {
            for (ConfigItem& item : section.items) {
                item.currentValue = item.defaultValue;
                emit configValueChanged(domain, extension, section.key, item.key,
                                        item.defaultValue);
            }
            break;
        }
    }

    saveExtensionConfig(domain, extension);
}

void ConfigManager::resetItemToDefault(const QString& domain, const QString& extension,
                                       const QString& sectionKey, const QString& itemKey) {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        return;
    }

    ConfigPage& page = config_pages_[pageKey];
    for (ConfigSection& section : page.sections) {
        if (section.key == sectionKey) {
            for (ConfigItem& item : section.items) {
                if (item.key == itemKey) {
                    item.currentValue = item.defaultValue;
                    emit configValueChanged(domain, extension, section.key, item.key,
                                            item.defaultValue);
                    saveExtensionConfig(domain, extension);
                    return;
                }
            }
        }
    }
}

bool ConfigManager::save() {
    bool allSuccess = true;
    for (const ConfigPage& page : config_pages_.values()) {
        if (!saveExtensionConfig(page.domain, page.extension)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool ConfigManager::load() {
    bool allSuccess = true;
    for (const ConfigPage& page : config_pages_.values()) {
        if (!loadExtensionConfig(page.domain, page.extension)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool ConfigManager::saveExtensionConfig(const QString& domain, const QString& extension) {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        return false;
    }

    QString filePath = getConfigFilePath(domain, extension);
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    const ConfigPage& page = config_pages_[pageKey];

    // Build JSON structure
    QJsonObject root;
    root["domain"] = domain;
    root["extension"] = extension;
    root["version"] = "1.0";

    QJsonObject sectionsObj;
    for (const ConfigSection& section : page.sections) {
        QJsonObject itemsObj;
        for (const ConfigItem& item : section.items) {
            if (item.currentValue.isValid()) {
                itemsObj[item.key] = QJsonValue::fromVariant(item.currentValue);
            }
        }
        sectionsObj[section.key] = itemsObj;
    }
    root["config"] = sectionsObj;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save config:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Saved config:" << filePath;
    return true;
}

bool ConfigManager::loadExtensionConfig(const QString& domain, const QString& extension) {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        return false;
    }

    QString filePath = getConfigFilePath(domain, extension);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No saved config found for:" << pageKey;
        return false;  // Not an error, just no saved config
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        qWarning() << "Invalid config file:" << filePath;
        return false;
    }

    QJsonObject root = doc.object();
    QJsonObject sectionsObj = root["config"].toObject();

    ConfigPage& page = config_pages_[pageKey];
    for (ConfigSection& section : page.sections) {
        if (!sectionsObj.contains(section.key)) {
            continue;
        }

        QJsonObject itemsObj = sectionsObj[section.key].toObject();
        for (ConfigItem& item : section.items) {
            if (itemsObj.contains(item.key)) {
                item.currentValue = itemsObj[item.key].toVariant();
            }
        }
    }

    qDebug() << "Loaded config:" << filePath;
    return true;
}

void ConfigManager::setComplexityLevel(ConfigComplexity level) {
    if (current_complexity_ != level) {
        current_complexity_ = level;
        emit complexityLevelChanged(level);
        qInfo() << "Config complexity level changed to:" << configComplexityToString(level);
    }
}

ConfigComplexity ConfigManager::getComplexityLevel() const {
    return current_complexity_;
}

QString ConfigManager::getConfigFilePath(const QString& domain, const QString& extension) const {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configPath + "/CrankshaftReborn/config/" + domain + "." + extension + ".json";
}

QString ConfigManager::makeKey(const QString& domain, const QString& extension) const {
    return domain + "." + extension;
}

bool ConfigManager::parseFullPath(const QString& fullPath, QString& domain, QString& extension,
                                  QString& section, QString& key) const {
    QStringList parts = fullPath.split('.');
    if (parts.size() < 4) {
        qWarning() << "Invalid config path:" << fullPath;
        return false;
    }

    domain = parts[0];
    extension = parts[1];
    section = parts[2];
    key = parts.mid(3).join('.');  // Allow dots in key names

    return true;
}

QVariantMap ConfigManager::exportConfig(bool maskSecrets) const {
    QVariantMap root;
    root["version"] = "1.0";
    root["exportDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["maskSecrets"] = maskSecrets;

    QVariantList pages;
    for (const ConfigPage& page : config_pages_.values()) {
        pages.append(exportConfigPage(page, maskSecrets));
    }
    root["pages"] = pages;

    return root;
}

QVariantMap ConfigManager::exportConfig(const QStringList& domainExtensions,
                                        bool maskSecrets) const {
    QVariantMap root;
    root["version"] = "1.0";
    root["exportDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["maskSecrets"] = maskSecrets;

    QVariantList pages;
    for (const QString& domainExtension : domainExtensions) {
        if (config_pages_.contains(domainExtension)) {
            pages.append(exportConfigPage(config_pages_[domainExtension], maskSecrets));
        }
    }
    root["pages"] = pages;

    return root;
}

bool ConfigManager::importConfig(const QVariantMap& config, bool overwriteExisting) {
    if (config.value("version").toString() != "1.0") {
        qWarning() << "Unsupported config version:" << config.value("version").toString();
        return false;
    }

    QVariantList pages = config.value("pages").toList();
    bool allSuccess = true;

    for (const QVariant& pageVar : pages) {
        QVariantMap pageData = pageVar.toMap();
        QString domain = pageData.value("domain").toString();
        QString extension = pageData.value("extension").toString();

        if (!importConfigPage(domain, extension, pageData, overwriteExisting)) {
            allSuccess = false;
        }
    }

    return allSuccess;
}

bool ConfigManager::importConfig(const QVariantMap& config, const QStringList& domainExtensions,
                                 bool overwriteExisting) {
    if (config.value("version").toString() != "1.0") {
        qWarning() << "Unsupported config version:" << config.value("version").toString();
        return false;
    }

    QVariantList pages = config.value("pages").toList();
    bool allSuccess = true;

    for (const QVariant& pageVar : pages) {
        QVariantMap pageData = pageVar.toMap();
        QString domain = pageData.value("domain").toString();
        QString extension = pageData.value("extension").toString();
        QString key = makeKey(domain, extension);

        if (!domainExtensions.contains(key)) {
            continue;  // Skip extensions not in the filter list
        }

        if (!importConfigPage(domain, extension, pageData, overwriteExisting)) {
            allSuccess = false;
        }
    }

    return allSuccess;
}

bool ConfigManager::backupToFile(const QString& filePath, bool maskSecrets, bool compress) {
    QVariantMap config = exportConfig(maskSecrets);

    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    if (compress) {
        return compressToFile(data, filePath);
    } else {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Failed to open file for backup:" << filePath;
            return false;
        }
        file.write(data);
        file.close();
        qInfo() << "Backed up config to:" << filePath;
        return true;
    }
}

bool ConfigManager::backupToFile(const QString& filePath, const QStringList& domainExtensions,
                                 bool maskSecrets, bool compress) {
    QVariantMap config = exportConfig(domainExtensions, maskSecrets);

    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    if (compress) {
        return compressToFile(data, filePath);
    } else {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Failed to open file for backup:" << filePath;
            return false;
        }
        file.write(data);
        file.close();
        qInfo() << "Backed up config to:" << filePath;
        return true;
    }
}

bool ConfigManager::restoreFromFile(const QString& filePath, bool overwriteExisting) {
    bool success = false;
    QByteArray data;

    // Try to decompress first (handles both compressed and uncompressed)
    if (filePath.endsWith(".gz")) {
        data = decompressFromFile(filePath, success);
        if (!success) {
            return false;
        }
    } else {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open file for restore:" << filePath;
            return false;
        }
        data = file.readAll();
        file.close();
        success = true;
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "Invalid config file format:" << filePath;
        return false;
    }

    QVariantMap config = doc.object().toVariantMap();
    bool result = importConfig(config, overwriteExisting);

    if (result) {
        qInfo() << "Restored config from:" << filePath;
    }

    return result;
}

bool ConfigManager::restoreFromFile(const QString& filePath, const QStringList& domainExtensions,
                                    bool overwriteExisting) {
    bool success = false;
    QByteArray data;

    // Try to decompress first (handles both compressed and uncompressed)
    if (filePath.endsWith(".gz")) {
        data = decompressFromFile(filePath, success);
        if (!success) {
            return false;
        }
    } else {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open file for restore:" << filePath;
            return false;
        }
        data = file.readAll();
        file.close();
        success = true;
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "Invalid config file format:" << filePath;
        return false;
    }

    QVariantMap config = doc.object().toVariantMap();
    bool result = importConfig(config, domainExtensions, overwriteExisting);

    if (result) {
        qInfo() << "Restored config from:" << filePath << "for extensions:" << domainExtensions;
    }

    return result;
}

bool ConfigManager::applyConfigChanges(const QVariantMap& config, bool overwriteExisting) {
    // Migration helper - same as import but with specific intent
    return importConfig(config, overwriteExisting);
}

QVariantMap ConfigManager::exportConfigPage(const ConfigPage& page, bool maskSecrets) const {
    QVariantMap pageMap;
    pageMap["domain"] = page.domain;
    pageMap["extension"] = page.extension;
    pageMap["title"] = page.title;

    QVariantMap sectionsMap;
    for (const ConfigSection& section : page.sections) {
        QVariantMap itemsMap;
        for (const ConfigItem& item : section.items) {
            QVariant value = item.currentValue.isValid() ? item.currentValue : item.defaultValue;

            // Mask secrets if requested
            if (maskSecrets && item.isSecret && value.isValid()) {
                itemsMap[item.key] = "***MASKED***";
            } else {
                itemsMap[item.key] = value;
            }
        }
        sectionsMap[section.key] = itemsMap;
    }
    pageMap["config"] = sectionsMap;

    return pageMap;
}

bool ConfigManager::importConfigPage(const QString& domain, const QString& extension,
                                     const QVariantMap& pageData, bool overwriteExisting) {
    QString pageKey = makeKey(domain, extension);
    if (!config_pages_.contains(pageKey)) {
        qWarning() << "Cannot import config for unregistered extension:" << pageKey;
        return false;
    }

    QVariantMap sectionsMap = pageData.value("config").toMap();
    ConfigPage& page = config_pages_[pageKey];

    for (ConfigSection& section : page.sections) {
        if (!sectionsMap.contains(section.key)) {
            continue;
        }

        QVariantMap itemsMap = sectionsMap[section.key].toMap();
        for (ConfigItem& item : section.items) {
            if (!itemsMap.contains(item.key)) {
                continue;
            }

            QVariant importedValue = itemsMap[item.key];

            // Skip masked secrets
            if (importedValue.toString() == "***MASKED***") {
                qDebug() << "Skipping masked secret:" << domain << extension << section.key
                         << item.key;
                continue;
            }

            // Skip if not overwriting and value already set
            if (!overwriteExisting && item.currentValue.isValid()) {
                qDebug() << "Skipping existing value:" << domain << extension << section.key
                         << item.key;
                continue;
            }

            // Set the value
            item.currentValue = importedValue;
            emit configValueChanged(domain, extension, section.key, item.key, importedValue);
        }
    }

    saveExtensionConfig(domain, extension);
    qInfo() << "Imported config for:" << pageKey;
    return true;
}

bool ConfigManager::compressToFile(const QByteArray& data, const QString& filePath) {
    QByteArray compressed = qCompress(data, 9);  // Maximum compression

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for compressed backup:" << filePath;
        return false;
    }

    file.write(compressed);
    file.close();

    const QString backupMsg = QString(
                                  "Backed up config to compressed file: %1 "
                                  "Original: %2 bytes "
                                  "Compressed: %3 bytes")
                                  .arg(filePath)
                                  .arg(data.size())
                                  .arg(compressed.size());
    qInfo() << backupMsg;
    return true;
}

QByteArray ConfigManager::decompressFromFile(const QString& filePath, bool& success) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for decompression:" << filePath;
        success = false;
        return QByteArray();
    }

    QByteArray compressed = file.readAll();
    file.close();

    QByteArray decompressed = qUncompress(compressed);
    if (decompressed.isEmpty() && !compressed.isEmpty()) {
        qWarning() << "Failed to decompress file:" << filePath;
        success = false;
        return QByteArray();
    }

    const QString decompressMsg = QString(
                                      "Decompressed config from: %1 "
                                      "Compressed: %2 bytes "
                                      "Decompressed: %3 bytes")
                                      .arg(filePath)
                                      .arg(compressed.size())
                                      .arg(decompressed.size());
    qDebug() << decompressMsg;

    success = true;
    return decompressed;
}

}  // namespace config
}  // namespace core
}  // namespace crankshaft
}  // namespace opencardev
