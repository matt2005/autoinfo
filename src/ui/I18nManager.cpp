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

#include "I18nManager.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QQmlEngine>
#include "../extensions/extension_manager.hpp"

namespace opencardev {
namespace crankshaft {
namespace ui {

I18nManager* I18nManager::self_ = nullptr;

I18nManager::I18nManager(QObject* parent)
    : QObject(parent), engine_(nullptr), ext_manager_(nullptr) {}

void I18nManager::registerQmlType() {
    qmlRegisterSingletonInstance("CrankshaftReborn.I18n", 1, 0, "I18nManager",
                                 I18nManager::instance());
}

I18nManager* I18nManager::instance() {
    if (!self_)
        self_ = new I18nManager();
    return self_;
}

void I18nManager::initialise(QQmlEngine* engine,
                             opencardev::crankshaft::extensions::ExtensionManager* extMgr) {
    auto* mgr = instance();
    mgr->engine_ = engine;
    mgr->ext_manager_ = extMgr;
}

QStringList I18nManager::availableLocales() const {
    // Discover .qm files in common locations. Use simple naming: core_<locale>.qm
    QStringList locales;
    QStringList searchPaths;
    searchPaths << (QCoreApplication::applicationDirPath() + "/i18n");
    searchPaths << QString::fromUtf8("/usr/share/CrankshaftReborn/i18n");
    searchPaths << QString::fromUtf8("/usr/share/crankshaft_reborn/i18n");

    for (const QString& base : searchPaths) {
        QDir dir(base);
        if (!dir.exists())
            continue;
        const QStringList files = dir.entryList(QStringList() << "core_*.qm", QDir::Files);
        for (const QString& f : files) {
            const QString loc = f.mid(QString("core_").length()).chopped(3);  // remove .qm
            if (!locales.contains(loc))
                locales << loc;
        }
    }
    // Ensure en_GB is present as default
    if (!locales.contains("en_GB"))
        locales.prepend("en_GB");
    return locales;
}

bool I18nManager::setLocale(const QString& locale) {
    if (current_locale_ == locale)
        return true;

    unloadTranslations();
    current_locale_ = locale;

    // Load core first
    if (!loadCoreTranslations(locale) && locale != QLatin1String("en_GB")) {
        // Fallback to British English if requested locale missing
        qWarning() << "I18n: falling back to en_GB for core translations";
        current_locale_ = "en_GB";
        loadCoreTranslations(current_locale_);
    }
    // Load extension translations if any
    reloadExtensionTranslations(current_locale_);

    // Retranslate QML
    if (engine_)
        engine_->retranslate();
    emit languageChanged(current_locale_);
    qInfo() << "I18n: language set to" << current_locale_;
    return true;
}

bool I18nManager::loadCoreTranslations(const QString& locale) {
    // Try applicationDir/i18n, then /usr/share...
    QStringList candidates;
    candidates << (QCoreApplication::applicationDirPath() + "/i18n/core_" + locale + ".qm");
    candidates << QString::fromUtf8("/usr/share/CrankshaftReborn/i18n/core_") + locale + ".qm";
    candidates << QString::fromUtf8("/usr/share/crankshaft_reborn/i18n/core_") + locale + ".qm";

    for (const QString& path : candidates) {
        if (QFileInfo::exists(path)) {
            if (core_translator_.load(path)) {
                QCoreApplication::installTranslator(&core_translator_);
                qInfo() << "I18n: loaded core translations from" << path;
                return true;
            }
        }
    }
    qDebug() << "I18n: no core .qm found for locale" << locale;
    return false;
}

void I18nManager::reloadExtensionTranslations(const QString& locale) {
    // Uninstall existing
    for (QTranslator* t : extension_translators_) {
        QCoreApplication::removeTranslator(t);
        delete t;
    }
    extension_translators_.clear();
    extensionLoadedLocale_.clear();
    extensionFallbackUsed_.clear();

    if (!ext_manager_)
        return;
    const QStringList ids = ext_manager_->getLoadedExtensions();
    for (const QString& id : ids) {
        bool primaryLoaded = loadExtensionTranslationFor(id, locale, /*allowFallback*/ true);
        if (!primaryLoaded && locale != QLatin1String("en_GB")) {
            // If fallback loaded, record
            if (extensionLoadedLocale_.value(id) == QLatin1String("en_GB")) {
                extensionFallbackUsed_.insert(id);
                emit translationFallbackOccurred(id);
            }
        }
    }
    emit extensionTranslationsChanged();
}

void I18nManager::unloadTranslations() {
    if (!core_translator_.isEmpty()) {
        QCoreApplication::removeTranslator(&core_translator_);
    }
    for (QTranslator* t : extension_translators_) {
        QCoreApplication::removeTranslator(t);
        delete t;
    }
    extension_translators_.clear();
}

void I18nManager::refreshTranslations() {
    // Re-apply current locale without unloading core translator if unchanged
    const QString loc = current_locale_.isEmpty() ? QStringLiteral("en_GB") : current_locale_;
    unloadTranslations();
    current_locale_ = loc;
    if (!loadCoreTranslations(loc) && loc != QLatin1String("en_GB")) {
        qWarning() << "I18n: refresh fallback to en_GB for core translations";
        current_locale_ = "en_GB";
        loadCoreTranslations(current_locale_);
    }
    reloadExtensionTranslations(current_locale_);
    if (engine_) {
        engine_->retranslate();
    }
    emit languageChanged(current_locale_);
    emit extensionTranslationsChanged();
}

bool I18nManager::loadExtensionTranslationFor(const QString& extensionId, const QString& locale,
                                              bool allowFallback) {
    const auto manifest = ext_manager_ ? ext_manager_->getManifest(extensionId) : decltype(ext_manager_->getManifest(extensionId))();
    Q_UNUSED(manifest); // Not yet used for path indirection

    QStringList candidates;
    const QString baseId = extensionId;
    // Development / build tree locations
    candidates << (QCoreApplication::applicationDirPath() + "/extensions/" + baseId + "/i18n/" + baseId + "_" + locale + ".qm");
    candidates << (QDir::currentPath() + "/extensions/" + baseId + "/i18n/" + baseId + "_" + locale + ".qm");
    // Installed system locations
    candidates << QString::fromUtf8("/usr/share/CrankshaftReborn/extensions/") + baseId + "/i18n/" + baseId + "_" + locale + ".qm";
    candidates << QString::fromUtf8("/usr/share/crankshaft_reborn/extensions/") + baseId + "/i18n/" + baseId + "_" + locale + ".qm";

    // Attempt primary locale
    for (const QString& p : candidates) {
        if (QFileInfo::exists(p)) {
            auto* tr = new QTranslator();
            if (tr->load(p)) {
                QCoreApplication::installTranslator(tr);
                extension_translators_.append(tr);
                qInfo() << "I18n: loaded extension translations for" << baseId << "locale" << locale << "from" << p;
                extensionLoadedLocale_.insert(baseId, locale);
                return true;
            }
            delete tr;
        }
    }

    if (allowFallback && locale != QLatin1String("en_GB")) {
        // Try fallback British English if specific locale missing
        const QString fallback = QStringLiteral("en_GB");
        for (QString p : candidates) {
            p.replace(locale + ".qm", fallback + ".qm");
            if (QFileInfo::exists(p)) {
                auto* tr = new QTranslator();
                if (tr->load(p)) {
                    QCoreApplication::installTranslator(tr);
                    extension_translators_.append(tr);
                    qInfo() << "I18n: fallback en_GB translation loaded for extension" << baseId << "from" << p;
                    extensionLoadedLocale_.insert(baseId, QStringLiteral("en_GB"));
                    return true;
                }
                delete tr;
            }
        }
    }
    qDebug() << "I18n: no translation for extension" << baseId << "(" << locale << ")";
    // If no translation loaded at all record empty
    if (!extensionLoadedLocale_.contains(baseId)) {
        extensionLoadedLocale_.insert(baseId, QString());
    }
    return false;
}

QStringList I18nManager::loadedExtensionIds() const {
    return extensionLoadedLocale_.keys();
}

QVariantList I18nManager::extensionTranslationStatus() const {
    QVariantList list;
    for (auto it = extensionLoadedLocale_.cbegin(); it != extensionLoadedLocale_.cend(); ++it) {
        const QString id = it.key();
        QVariantMap m;
        m[QStringLiteral("id")] = id;
        m[QStringLiteral("loadedLocale")] = it.value().isEmpty() ? QStringLiteral("(none)") : it.value();
        m[QStringLiteral("fallbackUsed")] = extensionFallbackUsed_.contains(id);
        list.append(m);
    }
    return list;
}

}  // namespace ui
}  // namespace crankshaft
}  // namespace opencardev
