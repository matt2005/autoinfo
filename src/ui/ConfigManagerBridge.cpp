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

#include "ConfigManagerBridge.hpp"
#include "../core/config/ConfigManager.hpp"
#include "../core/config/ConfigTypes.hpp"
#include <QDebug>

namespace opencardev { namespace crankshaft {
namespace ui {

ConfigManagerBridge* ConfigManagerBridge::instance_ = nullptr;

ConfigManagerBridge::ConfigManagerBridge(QObject* parent)
    : QObject(parent)
    , config_manager_(nullptr)
{
}

ConfigManagerBridge* ConfigManagerBridge::instance()
{
    if (instance_ == nullptr) {
        instance_ = new ConfigManagerBridge();
    }
    return instance_;
}

void ConfigManagerBridge::registerQmlType()
{
    qmlRegisterSingletonType<ConfigManagerBridge>(
        "Crankshaft.ConfigManagerBridge", 1, 0, "ConfigManagerBridge",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            return ConfigManagerBridge::instance();
        }
    );
}

void ConfigManagerBridge::initialise(core::config::ConfigManager* manager)
{
    if (instance_ == nullptr) {
        instance_ = new ConfigManagerBridge();
    }
    instance_->config_manager_ = manager;
    instance_->connectSignals();
    qDebug() << "ConfigManagerBridge initialised";
}

void ConfigManagerBridge::connectSignals()
{
    if (config_manager_ == nullptr) {
        return;
    }

        connect(config_manager_, &core::config::ConfigManager::configValueChanged,
            this, &ConfigManagerBridge::configValueChanged);
        connect(config_manager_, &core::config::ConfigManager::configPageRegistered,
            this, &ConfigManagerBridge::configPageRegistered);
        connect(config_manager_, &core::config::ConfigManager::complexityLevelChanged,
            this, [this](core::config::ConfigComplexity level) {
            emit complexityLevelChanged(core::config::configComplexityToString(level));
            });
}

QVariantList ConfigManagerBridge::getAllConfigPages() const
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return QVariantList();
    }

    QVariantList result;
    auto pages = config_manager_->getAllConfigPages();
    for (const auto& page : pages) {
        result.append(page.toMap());
    }
    return result;
}

QVariantList ConfigManagerBridge::getConfigPagesByDomain(const QString& domain) const
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return QVariantList();
    }

    QVariantList result;
    auto pages = config_manager_->getConfigPagesByDomain(domain);
    for (const auto& page : pages) {
        result.append(page.toMap());
    }
    return result;
}

QVariantMap ConfigManagerBridge::getConfigPage(const QString& domain, const QString& extension) const
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return QVariantMap();
    }

    auto page = config_manager_->getConfigPage(domain, extension);
    return page.toMap();
}

QVariant ConfigManagerBridge::getValue(const QString& domain, const QString& extension,
                                      const QString& section, const QString& key) const
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return QVariant();
    }

    return config_manager_->getValue(domain, extension, section, key);
}

QVariant ConfigManagerBridge::getValue(const QString& fullPath) const
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return QVariant();
    }

    return config_manager_->getValue(fullPath);
}

bool ConfigManagerBridge::setValue(const QString& domain, const QString& extension,
                                  const QString& section, const QString& key, const QVariant& value)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->setValue(domain, extension, section, key, value);
}

bool ConfigManagerBridge::setValue(const QString& fullPath, const QVariant& value)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->setValue(fullPath, value);
}

void ConfigManagerBridge::resetToDefaults(const QString& domain, const QString& extension)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return;
    }

    config_manager_->resetToDefaults(domain, extension);
}

void ConfigManagerBridge::resetSectionToDefaults(const QString& domain, const QString& extension,
                                                const QString& section)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return;
    }

    config_manager_->resetSectionToDefaults(domain, extension, section);
}

void ConfigManagerBridge::resetItemToDefault(const QString& domain, const QString& extension,
                                            const QString& section, const QString& key)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return;
    }

    config_manager_->resetItemToDefault(domain, extension, section, key);
}

bool ConfigManagerBridge::save()
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->save();
}

bool ConfigManagerBridge::load()
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->load();
}

QVariantMap ConfigManagerBridge::exportConfig(bool maskSecrets) const
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return QVariantMap();
    }

    return config_manager_->exportConfig(maskSecrets);
}

bool ConfigManagerBridge::importConfig(const QVariantMap& config, bool overwriteExisting)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->importConfig(config, overwriteExisting);
}

bool ConfigManagerBridge::backupToFile(const QString& filePath, bool maskSecrets, bool compress)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->backupToFile(filePath, maskSecrets, compress);
}

bool ConfigManagerBridge::backupToFile(const QString& filePath, const QStringList& domainExtensions,
                                      bool maskSecrets, bool compress)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->backupToFile(filePath, domainExtensions, maskSecrets, compress);
}

bool ConfigManagerBridge::restoreFromFile(const QString& filePath, bool overwriteExisting)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->restoreFromFile(filePath, overwriteExisting);
}

bool ConfigManagerBridge::restoreFromFile(const QString& filePath, const QStringList& domainExtensions,
                                         bool overwriteExisting)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return false;
    }

    return config_manager_->restoreFromFile(filePath, domainExtensions, overwriteExisting);
}

QString ConfigManagerBridge::getComplexityLevel() const
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return "Basic";
    }

    return core::config::configComplexityToString(config_manager_->getComplexityLevel());
}

void ConfigManagerBridge::setComplexityLevel(const QString& level)
{
    if (config_manager_ == nullptr) {
        qWarning() << "ConfigManager not initialised";
        return;
    }

    config_manager_->setComplexityLevel(core::config::stringToConfigComplexity(level));
}

QStringList ConfigManagerBridge::getComplexityLevels() const
{
    return QStringList{"Basic", "Advanced", "Expert", "Developer"};
}

}  // namespace ui
}  // namespace crankshaft
}  // namespace opencardev
