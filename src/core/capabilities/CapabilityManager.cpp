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

#include "CapabilityManager.hpp"
#include "LocationCapabilityImpl.hpp"
#include "NetworkCapabilityImpl.hpp"
#include "FileSystemCapabilityImpl.hpp"
#include "UICapabilityImpl.hpp"
#include "EventCapabilityImpl.hpp"
#include "AudioCapabilityImpl.hpp"
#include "TokenCapabilityImpl.hpp"
#include "WirelessCapabilityImpl.hpp"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QGeoPositionInfoSource>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTimer>
#include "../events/event_bus.hpp"
#include "../network/websocket_server.hpp"
#include "../ui/UIRegistrar.hpp"

// Forward declaration of Bluetooth capability factory to avoid header circular dependency.
namespace opencardev::crankshaft {
namespace core {
namespace capabilities {
class BluetoothCapability;  // forward declare type
std::shared_ptr<BluetoothCapability> createBluetoothCapabilityInstance(const QString& extensionId,
                                                                       CapabilityManager* manager);
}  // namespace capabilities
}  // namespace core
}  // namespace opencardev::crankshaft

namespace opencardev::crankshaft {
namespace core {

// LocationCapabilityImpl moved to LocationCapabilityImpl.{hpp,cpp}

// NetworkCapabilityImpl moved to NetworkCapabilityImpl.{hpp,cpp}

// AudioCapabilityImpl moved to AudioCapabilityImpl.{hpp,cpp}

// TokenCapabilityImpl moved to TokenCapabilityImpl.{hpp,cpp}

// FileSystemCapabilityImpl moved to FileSystemCapabilityImpl.{hpp,cpp}

// UICapabilityImpl moved to UICapabilityImpl.{hpp,cpp}

// EventCapabilityImpl moved to EventCapabilityImpl.{hpp,cpp}

// ============================================================================
// CapabilityManager Implementation
// ============================================================================

CapabilityManager::CapabilityManager(EventBus* event_bus, WebSocketServer* ws_server)
    : event_bus_(event_bus), ws_server_(ws_server) {}

CapabilityManager::~CapabilityManager() {
    // Invalidate all capabilities
    for (auto& extensionCaps : granted_capabilities_) {
        for (auto& cap : extensionCaps) {
            if (auto locCap = std::dynamic_pointer_cast<capabilities::LocationCapabilityImpl>(cap)) {
                locCap->invalidate();
            } else if (auto netCap = std::dynamic_pointer_cast<capabilities::NetworkCapabilityImpl>(cap)) {
                netCap->invalidate();
            } else if (auto fsCap = std::dynamic_pointer_cast<capabilities::FileSystemCapabilityImpl>(cap)) {
                fsCap->invalidate();
            } else if (auto uiCap = std::dynamic_pointer_cast<capabilities::UICapabilityImpl>(cap)) {
                uiCap->invalidate();
            } else if (auto evCap = std::dynamic_pointer_cast<capabilities::EventCapabilityImpl>(cap)) {
                evCap->invalidate();
            } else if (auto audCap = std::dynamic_pointer_cast<capabilities::AudioCapabilityImpl>(cap)) {
                audCap->invalidate();
            } else if (auto tokCap = std::dynamic_pointer_cast<capabilities::TokenCapabilityImpl>(cap)) {
                tokCap->invalidate();
            }
        }
    }
}

std::shared_ptr<capabilities::Capability> CapabilityManager::grantCapability(
    const QString& extensionId, const QString& capabilityType, const QVariantMap& options) {
    QMutexLocker locker(&mutex_);

    // Check if permission should be granted
    if (!shouldGrantPermission(extensionId, capabilityType, options)) {
        qWarning() << "Permission denied:" << extensionId << "requested" << capabilityType;
        return nullptr;
    }

    // Check if already granted
    if (granted_capabilities_.contains(extensionId) &&
        granted_capabilities_[extensionId].contains(capabilityType)) {
        return granted_capabilities_[extensionId][capabilityType];
    }

    // Create capability
    std::shared_ptr<capabilities::Capability> capability;

    if (capabilityType == "location") {
        capability = createLocationCapability(extensionId, options);
    } else if (capabilityType == "network") {
        capability = createNetworkCapability(extensionId, options);
    } else if (capabilityType == "filesystem") {
        capability = createFileSystemCapability(extensionId, options);
    } else if (capabilityType == "ui") {
        capability = createUICapability(extensionId, options);
    } else if (capabilityType == "event") {
        capability = createEventCapability(extensionId, options);
    } else if (capabilityType == "bluetooth") {
        capability = createBluetoothCapability(extensionId, options);
    } else if (capabilityType == "wireless") {
        capability = createWirelessCapability(extensionId, options);
    } else if (capabilityType == "audio") {
        capability = createAudioCapability(extensionId, options);
    } else if (capabilityType == "contacts" || capabilityType == "phone") {
        capability = createTokenCapability(extensionId, capabilityType);
    } else {
        qWarning() << "Unknown capability type:" << capabilityType;
        return nullptr;
    }

    if (capability) {
        granted_capabilities_[extensionId][capabilityType] = capability;

        qInfo() << "Granted capability:" << extensionId << "->" << capabilityType;

        logCapabilityUsage(extensionId, capabilityType, "granted", "");

        qDebug() << "Capability granted successfully, returning from grantCapability";
    }

    qDebug() << "Exiting grantCapability for:" << extensionId << capabilityType;
    return capability;
}

void CapabilityManager::revokeCapability(const QString& extensionId,
                                         const QString& capabilityType) {
    QMutexLocker locker(&mutex_);

    if (granted_capabilities_.contains(extensionId) &&
        granted_capabilities_[extensionId].contains(capabilityType)) {
        auto cap = granted_capabilities_[extensionId][capabilityType];

        // Invalidate capability
        if (auto locCap = std::dynamic_pointer_cast<capabilities::LocationCapabilityImpl>(cap)) {
            locCap->invalidate();
        } else if (auto netCap = std::dynamic_pointer_cast<capabilities::NetworkCapabilityImpl>(cap)) {
            netCap->invalidate();
        } else if (auto fsCap = std::dynamic_pointer_cast<capabilities::FileSystemCapabilityImpl>(cap)) {
            fsCap->invalidate();
        } else if (auto uiCap = std::dynamic_pointer_cast<capabilities::UICapabilityImpl>(cap)) {
            uiCap->invalidate();
        } else if (auto evCap = std::dynamic_pointer_cast<capabilities::EventCapabilityImpl>(cap)) {
            evCap->invalidate();
        } else if (auto audCap = std::dynamic_pointer_cast<capabilities::AudioCapabilityImpl>(cap)) {
            audCap->invalidate();
        } else if (auto tokCap = std::dynamic_pointer_cast<capabilities::TokenCapabilityImpl>(cap)) {
            tokCap->invalidate();
        }

        granted_capabilities_[extensionId].remove(capabilityType);

        logCapabilityUsage(extensionId, capabilityType, "revoked", "");

        qInfo() << "Revoked capability:" << extensionId << "->" << capabilityType;
    }
}

void CapabilityManager::revokeAllCapabilities(const QString& extensionId) {
    QMutexLocker locker(&mutex_);

    if (granted_capabilities_.contains(extensionId)) {
        for (const QString& capType : granted_capabilities_[extensionId].keys()) {
            auto cap = granted_capabilities_[extensionId][capType];

            // Invalidate
            if (auto locCap = std::dynamic_pointer_cast<capabilities::LocationCapabilityImpl>(cap)) {
                locCap->invalidate();
            } else if (auto netCap = std::dynamic_pointer_cast<capabilities::NetworkCapabilityImpl>(cap)) {
                netCap->invalidate();
            } else if (auto fsCap = std::dynamic_pointer_cast<capabilities::FileSystemCapabilityImpl>(cap)) {
                fsCap->invalidate();
            } else if (auto uiCap = std::dynamic_pointer_cast<capabilities::UICapabilityImpl>(cap)) {
                uiCap->invalidate();
            } else if (auto evCap = std::dynamic_pointer_cast<capabilities::EventCapabilityImpl>(cap)) {
                evCap->invalidate();
            }
        }

        granted_capabilities_.remove(extensionId);

        logCapabilityUsage(extensionId, "all", "revoked_all", "");

        qInfo() << "Revoked all capabilities for:" << extensionId;
    }
}

bool CapabilityManager::hasCapability(const QString& extensionId,
                                      const QString& capabilityType) const {
    QMutexLocker locker(&mutex_);

    return granted_capabilities_.contains(extensionId) &&
           granted_capabilities_[extensionId].contains(capabilityType) &&
           granted_capabilities_[extensionId][capabilityType]->isValid();
}

std::shared_ptr<capabilities::LocationCapability> CapabilityManager::getLocationCapability(
    const QString& extensionId) const {
    QMutexLocker locker(&mutex_);
    if (granted_capabilities_.contains(extensionId) &&
        granted_capabilities_[extensionId].contains("location")) {
        return std::dynamic_pointer_cast<capabilities::LocationCapability>(
            granted_capabilities_[extensionId]["location"]);
    }
    return nullptr;
}

void CapabilityManager::logCapabilityUsage(const QString& extensionId,
                                           const QString& capabilityType, const QString& action,
                                           const QString& details) {
    QMutexLocker locker(&mutex_);

    AuditLogEntry entry;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    entry.extension_id = extensionId;
    entry.capability_type = capabilityType;
    entry.action = action;
    entry.details = details;

    audit_log_.append(entry);

    // Keep log size reasonable (last 10000 entries)
    if (audit_log_.size() > 10000) {
        audit_log_.removeFirst();
    }
}

QList<QVariantMap> CapabilityManager::getAuditLog(const QString& extensionId, int limit) const {
    QMutexLocker locker(&mutex_);

    QList<QVariantMap> result;

    for (int i = audit_log_.size() - 1; i >= 0 && (limit == 0 || result.size() < limit); --i) {
        const auto& entry = audit_log_[i];

        if (!extensionId.isEmpty() && entry.extension_id != extensionId) {
            continue;
        }

        QVariantMap map;
        map["timestamp"] = entry.timestamp;
        map["extension_id"] = entry.extension_id;
        map["capability_type"] = entry.capability_type;
        map["action"] = entry.action;
        map["details"] = entry.details;

        result.append(map);
    }

    return result;
}

bool CapabilityManager::shouldGrantPermission(const QString& extensionId,
                                              const QString& capabilityType,
                                              const QVariantMap& options) const {
    // TODO: Check against manifest permissions
    // For now, grant all permissions (development mode)
    return true;
}

std::shared_ptr<capabilities::LocationCapability> CapabilityManager::createLocationCapability(
    const QString& extensionId, const QVariantMap& options) {
    return capabilities::createLocationCapabilityInstance(extensionId, this);
}

std::shared_ptr<capabilities::NetworkCapability> CapabilityManager::createNetworkCapability(
    const QString& extensionId, const QVariantMap& options) {
    return capabilities::createNetworkCapabilityInstance(extensionId, this);
}

std::shared_ptr<capabilities::FileSystemCapability> CapabilityManager::createFileSystemCapability(
    const QString& extensionId, const QVariantMap& options) {
    // Determine scope path
    QString scopePath = options.value("scope_path").toString();

    if (scopePath.isEmpty()) {
        // Default: $CACHE/extensions/{extension-id}/
        scopePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
                    "/extensions/" + extensionId;
    }

    return capabilities::createFileSystemCapabilityInstance(extensionId, this, scopePath);
}

std::shared_ptr<capabilities::UICapability> CapabilityManager::createUICapability(
    const QString& extensionId, const QVariantMap& options) {
    return capabilities::createUICapabilityInstance(extensionId, this);
}

std::shared_ptr<capabilities::EventCapability> CapabilityManager::createEventCapability(
    const QString& extensionId, const QVariantMap& options) {
    return capabilities::createEventCapabilityInstance(extensionId, this, event_bus_);
}

std::shared_ptr<capabilities::Capability> CapabilityManager::createBluetoothCapability(
    const QString& extensionId, const QVariantMap& options) {
    return std::static_pointer_cast<capabilities::Capability>(
        capabilities::createBluetoothCapabilityInstance(extensionId, this));
}

std::shared_ptr<capabilities::Capability> CapabilityManager::createWirelessCapability(
    const QString& extensionId, const QVariantMap& options) {
    Q_UNUSED(options);
    return std::static_pointer_cast<capabilities::Capability>(
        capabilities::createWirelessCapabilityInstance(extensionId));
}

std::shared_ptr<capabilities::AudioCapability> CapabilityManager::createAudioCapability(
    const QString& extensionId, const QVariantMap& options) {
    Q_UNUSED(options);
    return capabilities::createAudioCapabilityInstance(extensionId, this);
}

std::shared_ptr<capabilities::Capability> CapabilityManager::createTokenCapability(
    const QString& extensionId, const QString& capabilityId) {
    return std::static_pointer_cast<capabilities::Capability>(
        capabilities::createTokenCapabilityInstance(extensionId, capabilityId));
}

}  // namespace core
}  // namespace opencardev::crankshaft

// ===== CapabilityManager: UI registrar injection =====
namespace opencardev::crankshaft {
namespace core {

void CapabilityManager::setUIRegistrar(ui::UIRegistrar* registrar) {
    QMutexLocker locker(&mutex_);
    ui_registrar_ = registrar;
}

}  // namespace core
}  // namespace opencardev::crankshaft
