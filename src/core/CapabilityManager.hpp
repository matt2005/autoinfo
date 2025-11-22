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

#include "capabilities/Capability.hpp"
#include "capabilities/LocationCapability.hpp"
#include "capabilities/NetworkCapability.hpp"
#include "capabilities/FileSystemCapability.hpp"
#include "capabilities/UICapability.hpp"
#include "capabilities/EventCapability.hpp"
#include "capabilities/BluetoothCapability.hpp"
#include <QString>
#include <QVariantMap>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <memory>

namespace openauto {
namespace core {

// Forward declarations
class EventBus;
class WebSocketServer;
namespace ui { class UIRegistrar; }

/**
 * CapabilityManager grants, revokes, and audits capabilities for extensions.
 * 
 * This is the security gatekeeper - extensions cannot create capabilities
 * themselves, only the CapabilityManager can grant them based on:
 * - Manifest permissions
 * - User authorization
 * - System policies
 * 
 * All capability usage is logged for security auditing.
 */
class CapabilityManager {
public:
    explicit CapabilityManager(EventBus* event_bus, WebSocketServer* ws_server);
    ~CapabilityManager();
    
    /**
     * Grant a capability to an extension.
     * Checks manifest permissions and creates appropriate capability.
     * 
     * @param extensionId Extension requesting capability
     * @param capabilityType Type of capability ("location", "network", etc.)
     * @param options Optional capability-specific configuration
     * @return Capability pointer or nullptr if denied
     */
    std::shared_ptr<capabilities::Capability> grantCapability(
        const QString& extensionId,
        const QString& capabilityType,
        const QVariantMap& options = QVariantMap()
    );
    
    /**
     * Revoke a capability from an extension.
     * The capability object will become invalid.
     * 
     * @param extensionId Extension to revoke from
     * @param capabilityType Type of capability to revoke
     */
    void revokeCapability(const QString& extensionId, const QString& capabilityType);
    
    /**
     * Revoke all capabilities from an extension.
     * Called when extension stops or is unloaded.
     * 
     * @param extensionId Extension to revoke all capabilities from
     */
    void revokeAllCapabilities(const QString& extensionId);
    
    /**
     * Check if extension has a capability.
     * 
     * @param extensionId Extension to check
     * @param capabilityType Type of capability
     * @return true if extension has valid capability
     */
    bool hasCapability(const QString& extensionId, const QString& capabilityType) const;

    // Convenience accessor for location capability of an extension.
    std::shared_ptr<capabilities::LocationCapability> getLocationCapability(const QString& extensionId) const;
    
    /**
     * Log capability usage for security audit.
     * 
     * @param extensionId Extension using capability
     * @param capabilityType Type of capability
     * @param action Action performed (e.g., "getCurrentPosition", "get")
     * @param details Optional additional details
     */
    void logCapabilityUsage(
        const QString& extensionId,
        const QString& capabilityType,
        const QString& action,
        const QString& details = QString()
    );
    
    /**
     * Get audit log entries for an extension.
     * 
     * @param extensionId Extension to get logs for (empty = all extensions)
     * @param limit Maximum number of entries (0 = all)
     * @return List of audit log entries
     */
    QList<QVariantMap> getAuditLog(const QString& extensionId = QString(), int limit = 100) const;
    
    /**
     * Check if a permission should be granted based on manifest.
     * Override this to implement custom permission logic.
     * 
     * @param extensionId Extension requesting permission
     * @param capabilityType Type of capability
     * @param options Capability options
     * @return true if permission should be granted
     */
    virtual bool shouldGrantPermission(
        const QString& extensionId,
        const QString& capabilityType,
        const QVariantMap& options
    ) const;

    // Inject UI registrar implementation from UI module to decouple core from UI
    void setUIRegistrar(ui::UIRegistrar* registrar);
    ui::UIRegistrar* uiRegistrar() const { return ui_registrar_; }

private:
    // Factory methods for creating concrete capability implementations
    std::shared_ptr<capabilities::LocationCapability> createLocationCapability(
        const QString& extensionId,
        const QVariantMap& options
    );
    
    std::shared_ptr<capabilities::NetworkCapability> createNetworkCapability(
        const QString& extensionId,
        const QVariantMap& options
    );
    
    std::shared_ptr<capabilities::FileSystemCapability> createFileSystemCapability(
        const QString& extensionId,
        const QVariantMap& options
    );
    
    std::shared_ptr<capabilities::UICapability> createUICapability(
        const QString& extensionId,
        const QVariantMap& options
    );
    
    std::shared_ptr<capabilities::EventCapability> createEventCapability(
        const QString& extensionId,
        const QVariantMap& options
    );
    std::shared_ptr<capabilities::Capability> createBluetoothCapability(
        const QString& extensionId,
        const QVariantMap& options
    );
    
    // Core services (non-owned)
    EventBus* event_bus_;
    WebSocketServer* ws_server_;
    
    // Granted capabilities: extensionId -> (capabilityType -> capability)
    QMap<QString, QMap<QString, std::shared_ptr<capabilities::Capability>>> granted_capabilities_;
    
    // Audit log: list of usage events
    struct AuditLogEntry {
        qint64 timestamp;
        QString extension_id;
        QString capability_type;
        QString action;
        QString details;
    };
    QList<AuditLogEntry> audit_log_;
    
    // Thread safety
    mutable QRecursiveMutex mutex_;

    // Non-owned pointer to UI registrar (implemented in UI module)
    ui::UIRegistrar* ui_registrar_ = nullptr;
};

}  // namespace core
}  // namespace openauto
