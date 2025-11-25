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

#include "UICapability.hpp"
#include <QVariantMap>

namespace opencardev::crankshaft::core { class CapabilityManager; namespace ui { class UIRegistrar; } }

namespace opencardev::crankshaft::core::capabilities {

class UICapabilityImpl : public UICapability {
  public:
    UICapabilityImpl(const QString& extension_id, core::CapabilityManager* manager);
    QString extensionId() const override;
    bool isValid() const override;
    void invalidate();
    bool registerMainView(const QString& qmlPath, const QVariantMap& metadata) override;
    bool registerWidget(const QString& qmlPath, const QVariantMap& metadata) override;
    void showNotification(const QString& title, const QString& message, int duration, const QString& icon) override;
    void updateStatusBar(const QString& itemId, const QString& text, const QString& icon) override;
    void unregisterComponent(const QString& componentId) override;
  private:
    QString extension_id_;
    core::CapabilityManager* manager_;
    bool is_valid_;
};

inline std::shared_ptr<UICapability> createUICapabilityInstance(const QString& extensionId, core::CapabilityManager* mgr) {
    return std::static_pointer_cast<UICapability>(std::make_shared<UICapabilityImpl>(extensionId, mgr));
}

} // namespace opencardev::crankshaft::core::capabilities
