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

#include <QObject>
#include <QStringList>
#include <QTranslator>
#include <QList>
#include <QHash>
#include <QSet>
#include <QVariant>

class QQmlEngine;

namespace opencardev {
namespace crankshaft {
namespace extensions {
class ExtensionManager;
}
}  // namespace crankshaft
}  // namespace opencardev

namespace opencardev {
namespace crankshaft {
namespace ui {

class I18nManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentLocale READ currentLocale NOTIFY languageChanged)
  Q_PROPERTY(QVariantList extensionTranslationStatus READ extensionTranslationStatus NOTIFY extensionTranslationsChanged)

  public:
    static void registerQmlType();
    static I18nManager* instance();

    QString currentLocale() const { return current_locale_; }

    Q_INVOKABLE QStringList availableLocales() const;
    Q_INVOKABLE bool setLocale(const QString& locale);
    Q_INVOKABLE void refreshTranslations();  // Reload current locale (e.g. after extensions added)
    Q_INVOKABLE QStringList loadedExtensionIds() const;
    Q_INVOKABLE QVariantList extensionTranslationStatus() const; // list of { id, loadedLocale, fallbackUsed }

    // Wiring helpers
    static void initialise(QQmlEngine* engine,
                           opencardev::crankshaft::extensions::ExtensionManager* extMgr);

  signals:
    void languageChanged(const QString& newLocale);
    void extensionTranslationsChanged();
    void translationFallbackOccurred(const QString& extensionId);

  private:
    explicit I18nManager(QObject* parent = nullptr);
    bool loadCoreTranslations(const QString& locale);
    void unloadTranslations();
    void reloadExtensionTranslations(const QString& locale);
    bool loadExtensionTranslationFor(const QString& extensionId, const QString& locale,
                     bool allowFallback);

    static I18nManager* self_;
    QQmlEngine* engine_;
    opencardev::crankshaft::extensions::ExtensionManager* ext_manager_;
    QString current_locale_;
    QTranslator core_translator_;
    QList<QTranslator*> extension_translators_;
    // Diagnostics tracking
    QHash<QString, QString> extensionLoadedLocale_; // extensionId -> locale actually loaded
    QSet<QString> extensionFallbackUsed_;            // extensionIds that used fallback
};

}  // namespace ui
}  // namespace crankshaft
}  // namespace opencardev
