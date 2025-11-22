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

#pragma once

#include <QString>
#include <QVariantMap>

namespace opencardev::crankshaft {
namespace core {
namespace ui {

/**
 * Interface for registering UI components from core without depending on UI library.
 * Implemented in UI module and injected into CapabilityManager at runtime.
 */
class UIRegistrar {
public:
    virtual ~UIRegistrar() = default;

    virtual void registerComponent(
        const QString& extensionId,
        const QString& slotType,
        const QString& qmlPath,
        const QVariantMap& metadata
    ) = 0;

    virtual void unregisterComponent(const QString& componentId) = 0;
};

} // namespace ui
} // namespace core
} // namespace openauto
