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

  public:
    static void registerQmlType();
    static I18nManager* instance();

    QString currentLocale() const { return current_locale_; }

    Q_INVOKABLE QStringList availableLocales() const;
    Q_INVOKABLE bool setLocale(const QString& locale);

    // Wiring helpers
    static void initialise(QQmlEngine* engine,
                           opencardev::crankshaft::extensions::ExtensionManager* extMgr);

  signals:
    void languageChanged(const QString& newLocale);

  private:
    explicit I18nManager(QObject* parent = nullptr);
    bool loadCoreTranslations(const QString& locale);
    void unloadTranslations();
    void reloadExtensionTranslations(const QString& locale);

    static I18nManager* self_;
    QQmlEngine* engine_;
    opencardev::crankshaft::extensions::ExtensionManager* ext_manager_;
    QString current_locale_;
    QTranslator core_translator_;
    QList<QTranslator*> extension_translators_;
};

}  // namespace ui
}  // namespace crankshaft
}  // namespace opencardev
