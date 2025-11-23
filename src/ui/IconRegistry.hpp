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

#ifndef CRANKSHAFT_UI_ICON_REGISTRY_HPP
#define CRANKSHAFT_UI_ICON_REGISTRY_HPP

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace opencardev::crankshaft::ui {

class IconRegistry : public QObject {
    Q_OBJECT
public:
    static IconRegistry* instance();
    static void registerQmlType();

    Q_INVOKABLE QUrl iconUrl(const QString& name) const; // returns qrc:/ url for given icon name
    Q_INVOKABLE bool exists(const QString& name) const;
    Q_INVOKABLE QStringList listAvailable() const { return available_; }

private:
    explicit IconRegistry(QObject* parent = nullptr);
    void buildIndex();
    QString normalise(const QString& name) const;

    QStringList available_;
};

} // namespace opencardev::crankshaft::ui

#endif // CRANKSHAFT_UI_ICON_REGISTRY_HPP
