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

#include "ExtensionRegistry.hpp"
#include "../extensions/extension_manager.hpp"
#include <QQmlEngine>
#include <QDebug>

namespace opencardev::crankshaft {
namespace ui {

ExtensionRegistry* ExtensionRegistry::instance_ = nullptr;

ExtensionRegistry::ExtensionRegistry(
    extensions::ExtensionManager* extensionManager,
    QObject* parent
)
    : QObject(parent)
    , extension_manager_(extensionManager)
    , next_component_id_(1)
{
    if (instance_ == nullptr) {
        instance_ = this;
    }
}

ExtensionRegistry* ExtensionRegistry::instance() {
    return instance_;
}

void ExtensionRegistry::registerQmlType() {
    qmlRegisterSingletonType<ExtensionRegistry>(
        "CrankshaftReborn.Extensions",
        1,
        0,
        "ExtensionRegistry",
        ExtensionRegistry::qmlInstance
    );
}

QObject* ExtensionRegistry::qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine) {
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    
    if (instance_ == nullptr) {
        qWarning() << "ExtensionRegistry instance not created yet!";
        return nullptr;
    }
    
    // QML engine takes ownership
    QQmlEngine::setObjectOwnership(instance_, QQmlEngine::CppOwnership);
    return instance_;
}

void ExtensionRegistry::registerComponent(
    const QString& extensionId,
    const QString& slotType,
    const QString& qmlPath,
    const QVariantMap& metadata
) {
    ComponentInfo info;
    info.component_id = QString("%1_%2").arg(extensionId).arg(next_component_id_++);
    info.extension_id = extensionId;
    info.slot_type = slotType;
    info.qml_path = qmlPath;
    info.metadata = metadata;
    info.metadata["componentId"] = info.component_id;
    info.metadata["extensionId"] = extensionId;
    info.metadata["qmlPath"] = qmlPath;
    info.metadata["slotType"] = slotType;
    
    components_.append(info);
    
    qInfo() << "ExtensionRegistry: Registered component"
            << info.component_id << "from" << extensionId
            << "(" << slotType << ")";
    
    emit componentRegistered(extensionId, info.component_id);
    emit componentCountChanged();
    
    if (slotType == "main") {
        emit mainComponentsChanged();
    } else if (slotType == "widget") {
        emit widgetsChanged();
    }
}

void ExtensionRegistry::unregisterComponent(const QString& componentId) {
    for (int i = 0; i < components_.size(); ++i) {
        if (components_[i].component_id == componentId) {
            QString slotType = components_[i].slot_type;
            components_.removeAt(i);
            
            qInfo() << "ExtensionRegistry: Unregistered component" << componentId;
            
            emit componentUnregistered(componentId);
            emit componentCountChanged();
            
            if (slotType == "main") {
                emit mainComponentsChanged();
            } else if (slotType == "widget") {
                emit widgetsChanged();
            }
            
            break;
        }
    }
}

QVariantList ExtensionRegistry::mainComponents() const {
    QVariantList result;
    
    for (const auto& component : components_) {
        if (component.slot_type == "main") {
            result.append(component.metadata);
        }
    }
    
    return result;
}

QVariantList ExtensionRegistry::widgets() const {
    QVariantList result;
    
    for (const auto& component : components_) {
        if (component.slot_type == "widget") {
            result.append(component.metadata);
        }
    }
    
    return result;
}

int ExtensionRegistry::componentCount() const {
    return components_.size();
}

QVariantMap ExtensionRegistry::getComponent(const QString& componentId) const {
    for (const auto& component : components_) {
        if (component.component_id == componentId) {
            return component.metadata;
        }
    }
    
    return QVariantMap();
}

}  // namespace ui
}  // namespace opencardev::crankshaft
