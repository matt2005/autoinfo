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

#include <memory>
#include "../../src/core/capabilities/EventCapability.hpp"
#include "../../src/core/capabilities/UICapability.hpp"
#include "../../src/extensions/extension.hpp"

namespace opencardev::crankshaft {
namespace extensions {
namespace dialer {

class DialerExtension : public Extension {
  public:
    DialerExtension() = default;
    ~DialerExtension() override = default;

    // Lifecycle
    bool initialize() override;
    void start() override;
    void stop() override;
    void cleanup() override;

    // Metadata
    QString id() const override { return "dialer"; }
    QString name() const override { return "Dialler"; }
    QString version() const override { return "1.0.0"; }
    ExtensionType type() const override { return ExtensionType::UI; }

    // Configuration
    void registerConfigItems(core::config::ConfigManager* manager) override;

  private:
    void setupEventHandlers();

    std::shared_ptr<core::capabilities::EventCapability> eventCap_;
};

}  // namespace dialer
}  // namespace extensions
}  // namespace opencardev::crankshaft
