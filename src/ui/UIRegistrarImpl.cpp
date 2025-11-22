/*
[]: #  * Project: Crankshaft
[]: #  * This file is part of Crankshaft project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  Crankshaft is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  Crankshaft is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
*/

#include "UIRegistrarImpl.hpp"
#include "ExtensionRegistry.hpp"
#include <QDebug>

namespace openauto {
namespace ui {

void UIRegistrarImpl::registerComponent(
    const QString& extensionId,
    const QString& slotType,
    const QString& qmlPath,
    const QVariantMap& metadata
) {
    auto* reg = ExtensionRegistry::instance();
    if (!reg) {
        qWarning() << "ExtensionRegistry not initialised; cannot register" << extensionId << slotType << qmlPath;
        return;
    }
    reg->registerComponent(extensionId, slotType, qmlPath, metadata);
}

void UIRegistrarImpl::unregisterComponent(const QString& componentId) {
    auto* reg = ExtensionRegistry::instance();
    if (!reg) {
        qWarning() << "ExtensionRegistry not initialised; cannot unregister" << componentId;
        return;
    }
    reg->unregisterComponent(componentId);
}

} // namespace ui
} // namespace openauto
