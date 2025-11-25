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
#include "TokenCapabilityImpl.hpp"

using namespace opencardev::crankshaft::core::capabilities;

TokenCapabilityImpl::TokenCapabilityImpl(const QString& extension_id, const QString& cap_id)
    : extension_id_(extension_id), cap_id_(cap_id), valid_(true) {}

QString TokenCapabilityImpl::id() const { return cap_id_; }
bool TokenCapabilityImpl::isValid() const { return valid_; }
QString TokenCapabilityImpl::extensionId() const { return extension_id_; }
void TokenCapabilityImpl::invalidate() { valid_ = false; }
