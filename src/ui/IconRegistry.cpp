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

#include "IconRegistry.hpp"
#include <QDir>
#include <QQmlEngine>
#include <QDebug>

namespace opencardev::crankshaft::ui {

static IconRegistry* g_instance = nullptr;

IconRegistry* IconRegistry::instance() {
    if (!g_instance) {
        g_instance = new IconRegistry();
    }
    return g_instance;
}

IconRegistry::IconRegistry(QObject* parent) : QObject(parent) {
    buildIndex();
}

void IconRegistry::registerQmlType() {
    qmlRegisterSingletonInstance("CrankshaftReborn.UI", 1, 0, "IconRegistry", IconRegistry::instance());
}

void IconRegistry::buildIndex() {
    // Scan compiled resources under /icons/mdi
    QDir dir(":/icons/mdi");
    const QStringList files = dir.entryList(QStringList() << "*.svg", QDir::Files);
    available_.clear();
    for (const QString& f : files) {
        QString base = f;
        base.remove('.'); // keep name with dot removed for direct match? We'll keep full without extension
    }
    for (const QString& f : files) {
        QString name = f;
        if (name.endsWith(".svg")) {
            name.chop(4);
        }
        available_.append(name);
    }
    available_.sort();
}

QString IconRegistry::normalise(const QString& name) const {
    QString n = name.trimmed();
    if (n.startsWith("mdi:")) {
        n = n.mid(4);
    }
    if (n.endsWith(".svg")) {
        n.chop(4);
    }
    return n;
}

QUrl IconRegistry::iconUrl(const QString& name) const {
    const QString n = normalise(name);
    if (!available_.contains(n)) {
        return QUrl();
    }
    return QUrl(QStringLiteral("qrc:/icons/mdi/%1.svg").arg(n));
}

bool IconRegistry::exists(const QString& name) const {
    return available_.contains(normalise(name));
}

} // namespace opencardev::crankshaft::ui
