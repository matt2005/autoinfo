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
#include <QFileInfo>

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
    QDir rootDir(":/icons");
    qDebug() << "IconRegistry: Root exists:" << rootDir.exists();
    qDebug() << "IconRegistry: Root entries:" << rootDir.entryList();
    
    QDir dir(":/icons/mdi");
    qDebug() << "IconRegistry: mdi dir exists:" << dir.exists();
    const QStringList files = dir.entryList(QStringList() << "*.svg", QDir::Files);
    qDebug() << "IconRegistry: Found" << files.size() << "icon files";
    available_.clear();
    for (const QString& f : files) {
        QString name = f;
        if (name.endsWith(".svg")) {
            name.chop(4);
        }
        available_.append(name);
    }
    available_.sort();
    qDebug() << "IconRegistry: Registered" << available_.size() << "icons:" << available_;
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

QString IconRegistry::extractNamespace(const QString& name, QString& outName) const {
    const int colon = name.indexOf(':');
    if (colon > 0) {
        outName = name.mid(colon + 1);
        return name.left(colon);
    }
    outName = name;
    return QStringLiteral("mdi"); // default namespace
}

QUrl IconRegistry::iconUrl(const QString& name) const {
    QString iconName;
    const QString ns = extractNamespace(name, iconName);
    
    // Check extension icons first
    const QString fullKey = ns + ":" + iconName;
    if (extensionIcons_.contains(fullKey)) {
        return extensionIcons_.value(fullKey);
    }
    
    // Check MDI built-in icons
    if (ns == "mdi") {
        const QString normalized = normalise(iconName);
        if (available_.contains(normalized)) {
            return QUrl(QStringLiteral("qrc:/icons/mdi/%1.svg").arg(normalized));
        }
    }
    
    // Fallback to placeholder
    return QUrl(QStringLiteral("qrc:/icons/mdi/placeholder.svg"));
}

bool IconRegistry::exists(const QString& name) const {
    QString iconName;
    const QString ns = extractNamespace(name, iconName);
    const QString fullKey = ns + ":" + iconName;
    
    if (extensionIcons_.contains(fullKey)) {
        return true;
    }
    
    if (ns == "mdi") {
        return available_.contains(normalise(iconName));
    }
    
    return false;
}

void IconRegistry::registerExtensionIcon(const QString& extensionId, const QString& iconName, const QUrl& iconUrl) {
    const QString key = extensionId + ":" + iconName;
    extensionIcons_[key] = iconUrl;
    qDebug() << "Registered extension icon:" << key << "->" << iconUrl;
}

void IconRegistry::unregisterExtensionIcons(const QString& extensionId) {
    const QString prefix = extensionId + ":";
    QMutableMapIterator<QString, QUrl> it(extensionIcons_);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith(prefix)) {
            qDebug() << "Unregistered extension icon:" << it.key();
            it.remove();
        }
    }
}

} // namespace opencardev::crankshaft::ui
