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

#include "Capability.hpp"

namespace opencardev::crankshaft::core::capabilities {

class TokenCapabilityImpl : public Capability {
  public:
    TokenCapabilityImpl(const QString& extension_id, const QString& cap_id);
    QString id() const override;
    bool isValid() const override;
    QString extensionId() const override;
    void invalidate();

  private:
    QString extension_id_;
    QString cap_id_;
    bool valid_;
};

inline std::shared_ptr<Capability> createTokenCapabilityInstance(const QString& extensionId,
                                                                 const QString& capabilityId) {
    return std::static_pointer_cast<Capability>(
        std::make_shared<TokenCapabilityImpl>(extensionId, capabilityId));
}

}  // namespace opencardev::crankshaft::core::capabilities
