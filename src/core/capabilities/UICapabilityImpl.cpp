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
#include "UICapabilityImpl.hpp"
#include <QDebug>
#include "../ui/UIRegistrar.hpp"
#include "CapabilityManager.hpp"

using namespace opencardev::crankshaft::core::capabilities;
using opencardev::crankshaft::core::CapabilityManager;

UICapabilityImpl::UICapabilityImpl(const QString& extension_id, CapabilityManager* manager)
    : extension_id_(extension_id), manager_(manager), is_valid_(true) {}

QString UICapabilityImpl::extensionId() const {
    return extension_id_;
}
bool UICapabilityImpl::isValid() const {
    return is_valid_;
}
void UICapabilityImpl::invalidate() {
    is_valid_ = false;
}

bool UICapabilityImpl::registerMainView(const QString& qmlPath, const QVariantMap& metadata) {
    if (!is_valid_)
        return false;
    manager_->logCapabilityUsage(extension_id_, "ui", "registerMainView", qmlPath);
    auto* registrar = manager_->uiRegistrar();
    if (!registrar) {
        qWarning() << "UIRegistrar not set; cannot register main view";
        return false;
    }
    registrar->registerComponent(extension_id_, "main", qmlPath, metadata);
    return true;
}

bool UICapabilityImpl::registerWidget(const QString& qmlPath, const QVariantMap& metadata) {
    if (!is_valid_)
        return false;
    manager_->logCapabilityUsage(extension_id_, "ui", "registerWidget", qmlPath);
    auto* registrar = manager_->uiRegistrar();
    if (!registrar) {
        qWarning() << "UIRegistrar not set; cannot register widget";
        return false;
    }
    registrar->registerComponent(extension_id_, "widget", qmlPath, metadata);
    return true;
}

void UICapabilityImpl::showNotification(const QString& title, const QString& message, int duration,
                                        const QString& icon) {
    if (!is_valid_)
        return;
    Q_UNUSED(duration);
    Q_UNUSED(icon);
    manager_->logCapabilityUsage(extension_id_, "ui", "showNotification",
                                 QString("%1: %2").arg(title, message));
}

void UICapabilityImpl::updateStatusBar(const QString& itemId, const QString& text,
                                       const QString& icon) {
    if (!is_valid_)
        return;
    Q_UNUSED(icon);
    manager_->logCapabilityUsage(extension_id_, "ui", "updateStatusBar",
                                 QString("%1: %2").arg(itemId, text));
}

void UICapabilityImpl::unregisterComponent(const QString& componentId) {
    if (!is_valid_)
        return;
    manager_->logCapabilityUsage(extension_id_, "ui", "unregisterComponent", componentId);
}
