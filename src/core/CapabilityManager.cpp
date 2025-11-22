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
#include "event_bus.hpp"
#include "websocket_server.hpp"
#include "ui/UIRegistrar.hpp"
#include <QGeoPositionInfoSource>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QStorageInfo>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>

// Forward declaration of Bluetooth capability factory to avoid header circular dependency.
namespace openauto { namespace core { namespace capabilities {
class BluetoothCapability; // forward declare type
std::shared_ptr<BluetoothCapability> createBluetoothCapabilityInstance(const QString& extensionId, CapabilityManager* manager);
}}}

namespace openauto {
namespace core {

// ============================================================================
// Concrete Capability Implementations
// ============================================================================

/**
 * Concrete LocationCapability implementation.
 */
class LocationCapabilityImpl : public capabilities::LocationCapability {
public:
    LocationCapabilityImpl(const QString& extension_id, CapabilityManager* manager)
        : extension_id_(extension_id)
        , manager_(manager)
        , is_valid_(true)
        , next_subscription_id_(1)
        , position_source_(nullptr)
        , device_mode_(DeviceMode::Internal)
        , mock_timer_(nullptr)
    {
        // Lazy initialization - don't start position source until actually used
        // This prevents hanging on systems without GPS
    }
    
    void ensurePositionSource() {
        if (!position_source_) {
            position_source_ = QGeoPositionInfoSource::createDefaultSource(nullptr);
            if (position_source_) {
                position_source_->startUpdates();
                QObject::connect(
                    position_source_,
                    &QGeoPositionInfoSource::positionUpdated,
                    [this](const QGeoPositionInfo& info) {
                        onPositionUpdated(info);
                    }
                );
                qDebug() << "Position source initialized for extension:" << extension_id_;
            } else {
                qWarning() << "Failed to create position source for extension:" << extension_id_;
            }
        }
    }
    
    ~LocationCapabilityImpl() override {
        if (position_source_) {
            position_source_->stopUpdates();
            delete position_source_;
        }
    }
    
    QString extensionId() const override { return extension_id_; }
    bool isValid() const override { return is_valid_; }
    
    void invalidate() { is_valid_ = false; }
    
    QGeoCoordinate getCurrentPosition() const override {
        if (!is_valid_) {
            return QGeoCoordinate();
        }
        // For mock modes return mock coordinate immediately.
        if (device_mode_ == DeviceMode::MockStatic || device_mode_ == DeviceMode::MockIP) {
            return mock_coordinate_;
        }

        const_cast<LocationCapabilityImpl*>(this)->ensurePositionSource();
        if (!position_source_) return QGeoCoordinate();
        
        manager_->logCapabilityUsage(extension_id_, "location", "getCurrentPosition");
        
        auto lastPos = position_source_->lastKnownPosition();
        return lastPos.coordinate();
    }
    
    int subscribeToUpdates(std::function<void(const QGeoCoordinate&)> callback) override {
        if (!is_valid_) {
            return -1;
        }
        
        ensurePositionSource();
        
        int id = next_subscription_id_++;
        subscriptions_[id] = callback;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "location",
            "subscribeToUpdates",
            QString("subscription_id=%1").arg(id)
        );
        
        return id;
    }
    
    void unsubscribe(int subscriptionId) override {
        subscriptions_.remove(subscriptionId);
        
        manager_->logCapabilityUsage(
            extension_id_,
            "location",
            "unsubscribe",
            QString("subscription_id=%1").arg(subscriptionId)
        );
    }
    
    double getAccuracy() const override {
        if (!is_valid_) return -1.0;
        if (device_mode_ == DeviceMode::MockStatic) return 25.0; // Approximate fixed accuracy
        if (device_mode_ == DeviceMode::MockIP) return 5000.0; // IP based coarse accuracy
        if (!position_source_) return -1.0;
        auto lastPos = position_source_->lastKnownPosition();
        return lastPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy) ? lastPos.attribute(QGeoPositionInfo::HorizontalAccuracy) : -1.0;
    }
    
    bool isAvailable() const override {
        if (!is_valid_) return false;
        if (device_mode_ == DeviceMode::MockStatic || device_mode_ == DeviceMode::MockIP) return true;
        return position_source_ != nullptr;
    }

    void setDeviceMode(DeviceMode mode) override {
        if (device_mode_ == mode) return;
        device_mode_ = mode;

        // Stop any existing sources
        if (position_source_) {
            position_source_->stopUpdates();
        }
        if (mock_timer_) {
            mock_timer_->stop();
        }

        if (mode == DeviceMode::MockStatic) {
            mock_coordinate_ = QGeoCoordinate(51.5074, -0.1278); // London static
            ensureMockTimer();
        } else if (mode == DeviceMode::MockIP) {
            // Perform IP geolocation lookup using simple JSON parsing.
            QNetworkAccessManager* nm = new QNetworkAccessManager();
            QObject::connect(nm, &QNetworkAccessManager::finished, [this, nm](QNetworkReply* reply) {
                if (reply->error() == QNetworkReply::NoError) {
                    const QByteArray data = reply->readAll();
                    QJsonParseError parseError; // requires include QJsonDocument/QJsonObject
                    QJsonDocument jd = QJsonDocument::fromJson(data, &parseError);
                    if (parseError.error == QJsonParseError::NoError && jd.isObject()) {
                        QJsonObject obj = jd.object();
                        double lat = obj.value("lat").toDouble();
                        double lon = obj.value("lon").toDouble();
                        if (lat != 0.0 || lon != 0.0) {
                            mock_coordinate_ = QGeoCoordinate(lat, lon);
                        } else {
                            mock_coordinate_ = QGeoCoordinate(51.5074, -0.1278); // Fallback
                        }
                    } else {
                        mock_coordinate_ = QGeoCoordinate(51.5074, -0.1278); // Fallback
                    }
                } else {
                    mock_coordinate_ = QGeoCoordinate(51.5074, -0.1278); // Fallback
                }
                reply->deleteLater();
                nm->deleteLater();
                ensureMockTimer();
            });
            nm->get(QNetworkRequest(QUrl("http://ip-api.com/json")));
        } else {
            // Real device modes
            ensurePositionSource();
            if (mock_timer_) mock_timer_->stop();
        }
    }

    DeviceMode deviceMode() const override { return device_mode_; }

private:
    void onPositionUpdated(const QGeoPositionInfo& info) {
        if (!is_valid_) return;
        
        QGeoCoordinate coord = info.coordinate();
        for (const auto& callback : subscriptions_) {
            callback(coord);
        }
    }

    void ensureMockTimer() {
        if (!mock_timer_) {
            mock_timer_ = new QTimer();
            QObject::connect(mock_timer_, &QTimer::timeout, [this]() {
                // For mock modes we emit current mock coordinate (optionally minor jitter)
                if (device_mode_ == DeviceMode::MockStatic || device_mode_ == DeviceMode::MockIP) {
                    for (const auto& callback : subscriptions_) {
                        callback(mock_coordinate_);
                    }
                }
            });
        }
        mock_timer_->start(5000);
    }
    
    QString extension_id_;
    CapabilityManager* manager_;
    bool is_valid_;
    QGeoPositionInfoSource* position_source_;
    QMap<int, std::function<void(const QGeoCoordinate&)>> subscriptions_;
    int next_subscription_id_;
    DeviceMode device_mode_;
    QTimer* mock_timer_;
    QGeoCoordinate mock_coordinate_;
};

/**
 * Concrete NetworkCapability implementation.
 */
class NetworkCapabilityImpl : public capabilities::NetworkCapability {
public:
    NetworkCapabilityImpl(const QString& extension_id, CapabilityManager* manager)
        : extension_id_(extension_id)
        , manager_(manager)
        , is_valid_(true)
        , network_manager_(new QNetworkAccessManager())
    {
    }
    
    ~NetworkCapabilityImpl() override {
        delete network_manager_;
    }
    
    QString extensionId() const override { return extension_id_; }
    bool isValid() const override { return is_valid_; }
    
    void invalidate() { is_valid_ = false; }
    
    QNetworkReply* get(const QUrl& url) override {
        if (!is_valid_) return nullptr;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "network",
            "get",
            url.toString()
        );
        
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, "CrankshaftReborn/1.0");
        return network_manager_->get(request);
    }
    
    QNetworkReply* post(const QUrl& url, const QByteArray& data) override {
        if (!is_valid_) return nullptr;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "network",
            "post",
            QString("%1 (%2 bytes)").arg(url.toString()).arg(data.size())
        );
        
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, "CrankshaftReborn/1.0");
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        return network_manager_->post(request, data);
    }
    
    QNetworkReply* put(const QUrl& url, const QByteArray& data) override {
        if (!is_valid_) return nullptr;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "network",
            "put",
            QString("%1 (%2 bytes)").arg(url.toString()).arg(data.size())
        );
        
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, "CrankshaftReborn/1.0");
        return network_manager_->put(request, data);
    }
    
    QNetworkReply* deleteResource(const QUrl& url) override {
        if (!is_valid_) return nullptr;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "network",
            "delete",
            url.toString()
        );
        
        QNetworkRequest request(url);
        return network_manager_->deleteResource(request);
    }
    
    QNetworkReply* downloadFile(const QUrl& url, const QString& localPath) override {
        if (!is_valid_) return nullptr;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "network",
            "downloadFile",
            QString("%1 -> %2").arg(url.toString(), localPath)
        );
        
        // TODO: Implement file download with progress tracking
        return get(url);
    }
    
    bool isOnline() const override {
        // Qt6 removed networkAccessible(), assume online if network_manager_ exists
        return is_valid_ && network_manager_ != nullptr;
    }

private:
    QString extension_id_;
    CapabilityManager* manager_;
    bool is_valid_;
    QNetworkAccessManager* network_manager_;
};

/**
 * Concrete FileSystemCapability implementation.
 */
class FileSystemCapabilityImpl : public capabilities::FileSystemCapability {
public:
    FileSystemCapabilityImpl(
        const QString& extension_id,
        CapabilityManager* manager,
        const QString& scope_path
    )
        : extension_id_(extension_id)
        , manager_(manager)
        , is_valid_(true)
        , scope_path_(scope_path)
    {
        // Ensure scope directory exists
        QDir dir;
        if (!dir.mkpath(scope_path_)) {
            qWarning() << "Failed to create filesystem scope:" << scope_path_;
        }
    }
    
    QString extensionId() const override { return extension_id_; }
    bool isValid() const override { return is_valid_; }
    
    void invalidate() { is_valid_ = false; }
    
    QFile* openFile(const QString& relativePath, QIODevice::OpenMode mode) override {
        if (!is_valid_) return nullptr;
        
        // Prevent path traversal attacks
        if (relativePath.contains("..") || relativePath.startsWith("/")) {
            qWarning() << "Rejected suspicious file path:" << relativePath;
            return nullptr;
        }
        
        QString absolutePath = QDir(scope_path_).filePath(relativePath);
        
        manager_->logCapabilityUsage(
            extension_id_,
            "filesystem",
            "openFile",
            QString("%1 (mode=%2)").arg(relativePath).arg((int)mode)
        );
        
        QFile* file = new QFile(absolutePath);
        if (!file->open(mode)) {
            qWarning() << "Failed to open file:" << absolutePath;
            delete file;
            return nullptr;
        }
        
        return file;
    }
    
    QDir scopedDirectory() const override {
        return QDir(scope_path_);
    }
    
    QStringList listFiles(const QStringList& nameFilters) const override {
        if (!is_valid_) return QStringList();
        
        QDir dir(scope_path_);
        QStringList files;
        
        QDirIterator it(
            scope_path_,
            nameFilters.isEmpty() ? QStringList{"*"} : nameFilters,
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories
        );
        
        while (it.hasNext()) {
            QString absPath = it.next();
            files << dir.relativeFilePath(absPath);
        }
        
        return files;
    }
    
    bool fileExists(const QString& relativePath) const override {
        if (!is_valid_ || relativePath.contains("..") || relativePath.startsWith("/")) {
            return false;
        }
        
        QString absolutePath = QDir(scope_path_).filePath(relativePath);
        return QFile::exists(absolutePath);
    }
    
    bool createDirectory(const QString& relativePath) override {
        if (!is_valid_ || relativePath.contains("..") || relativePath.startsWith("/")) {
            return false;
        }
        
        QString absolutePath = QDir(scope_path_).filePath(relativePath);
        
        manager_->logCapabilityUsage(
            extension_id_,
            "filesystem",
            "createDirectory",
            relativePath
        );
        
        return QDir().mkpath(absolutePath);
    }
    
    bool deleteFile(const QString& relativePath) override {
        if (!is_valid_ || relativePath.contains("..") || relativePath.startsWith("/")) {
            return false;
        }
        
        QString absolutePath = QDir(scope_path_).filePath(relativePath);
        
        manager_->logCapabilityUsage(
            extension_id_,
            "filesystem",
            "deleteFile",
            relativePath
        );
        
        return QFile::remove(absolutePath);
    }
    
    QString scopePath() const override {
        return scope_path_;
    }
    
    qint64 availableSpace() const override {
        return QStorageInfo(scope_path_).bytesAvailable();
    }

private:
    QString extension_id_;
    CapabilityManager* manager_;
    bool is_valid_;
    QString scope_path_;
};

/**
 * Concrete UICapability implementation.
 */
class UICapabilityImpl : public capabilities::UICapability {
public:
    UICapabilityImpl(const QString& extension_id, CapabilityManager* manager)
        : extension_id_(extension_id)
        , manager_(manager)
        , is_valid_(true)
    {
    }
    
    QString extensionId() const override { return extension_id_; }
    bool isValid() const override { return is_valid_; }
    
    void invalidate() { is_valid_ = false; }
    
    bool registerMainView(const QString& qmlPath, const QVariantMap& metadata) override {
        if (!is_valid_) return false;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "ui",
            "registerMainView",
            qmlPath
        );
        
        // Register via injected UI registrar to avoid core->UI dependency
        auto* registrar = manager_->uiRegistrar();
        if (registrar) {
            registrar->registerComponent(extension_id_, "main", qmlPath, metadata);
            qDebug() << "Registered main view for extension:" << extension_id_ << "at" << qmlPath;
        } else {
            qWarning() << "UIRegistrar not set; cannot register main view";
            return false;
        }
        
        return true;
    }
    
    bool registerWidget(const QString& qmlPath, const QVariantMap& metadata) override {
        if (!is_valid_) return false;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "ui",
            "registerWidget",
            qmlPath
        );
        
        // Register widget via injected UI registrar
        auto* registrar = manager_->uiRegistrar();
        if (registrar) {
            registrar->registerComponent(extension_id_, "widget", qmlPath, metadata);
            qDebug() << "Registered widget for extension:" << extension_id_ << "at" << qmlPath;
        } else {
            qWarning() << "UIRegistrar not set; cannot register widget";
            return false;
        }
        
        return true;
    }
    
    void showNotification(
        const QString& title,
        const QString& message,
        int duration,
        const QString& icon
    ) override {
        if (!is_valid_) return;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "ui",
            "showNotification",
            QString("%1: %2").arg(title, message)
        );
        
        // TODO: Emit event for notification system
    }
    
    void updateStatusBar(
        const QString& itemId,
        const QString& text,
        const QString& icon
    ) override {
        if (!is_valid_) return;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "ui",
            "updateStatusBar",
            QString("%1: %2").arg(itemId, text)
        );
    }
    
    void unregisterComponent(const QString& componentId) override {
        if (!is_valid_) return;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "ui",
            "unregisterComponent",
            componentId
        );
    }

private:
    QString extension_id_;
    CapabilityManager* manager_;
    bool is_valid_;
};

/**
 * Concrete EventCapability implementation.
 */
class EventCapabilityImpl : public capabilities::EventCapability {
public:
    EventCapabilityImpl(
        const QString& extension_id,
        CapabilityManager* manager,
        EventBus* event_bus
    )
        : extension_id_(extension_id)
        , manager_(manager)
        , event_bus_(event_bus)
        , is_valid_(true)
        , next_subscription_id_(1)
    {
    }
    
    QString extensionId() const override { return extension_id_; }
    bool isValid() const override { return is_valid_; }
    
    void invalidate() { 
        is_valid_ = false;
        // Unsubscribe all
        for (auto subId : subscriptions_.keys()) {
            event_bus_->unsubscribe(subId);
        }
        subscriptions_.clear();
    }
    
    bool emitEvent(const QString& eventName, const QVariantMap& eventData) override {
        if (!is_valid_ || !event_bus_) return false;
        
        // Prefix event name with extension namespace
        QString fullEventName = extension_id_ + "." + eventName;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "event",
            "emit",
            fullEventName
        );
        
        event_bus_->publish(fullEventName, eventData);
        return true;
    }
    
    int subscribe(
        const QString& eventPattern,
        std::function<void(const QVariantMap&)> callback
    ) override {
        if (!is_valid_ || !event_bus_) return -1;
        
        // Check if extension can subscribe to this pattern
        if (!canSubscribe(eventPattern)) {
            qWarning() << "Extension" << extension_id_ 
                       << "denied subscription to" << eventPattern;
            return -1;
        }
        
        int localId = next_subscription_id_++;
        int busId = event_bus_->subscribe(eventPattern, callback);
        subscriptions_[localId] = busId;
        
        manager_->logCapabilityUsage(
            extension_id_,
            "event",
            "subscribe",
            eventPattern
        );
        
        return localId;
    }
    
    void unsubscribe(int subscriptionId) override {
        if (!is_valid_ || !event_bus_) return;
        
        if (subscriptions_.contains(subscriptionId)) {
            event_bus_->unsubscribe(subscriptions_[subscriptionId]);
            subscriptions_.remove(subscriptionId);
            
            manager_->logCapabilityUsage(
                extension_id_,
                "event",
                "unsubscribe",
                QString::number(subscriptionId)
            );
        }
    }
    
    bool canEmit(const QString& eventName) const override {
        // Extensions can only emit events in their own namespace
        return eventName.startsWith(extension_id_ + ".");
    }
    
    bool canSubscribe(const QString& eventPattern) const override {
        // Extensions can subscribe to:
        // - Their own events (extension_id.*)
        // - Core public events (core.*)
        // - All events (*) if granted permission
        
        if (eventPattern.startsWith(extension_id_ + ".")) return true;
        if (eventPattern.startsWith("core.")) return true;
        if (eventPattern == "*" || eventPattern.startsWith("*.")) return true;
        
        return false;
    }

private:
    QString extension_id_;
    CapabilityManager* manager_;
    EventBus* event_bus_;
    bool is_valid_;
    QMap<int, int> subscriptions_; // local ID -> bus ID
    int next_subscription_id_;
};

// ============================================================================
// CapabilityManager Implementation
// ============================================================================

CapabilityManager::CapabilityManager(EventBus* event_bus, WebSocketServer* ws_server)
    : event_bus_(event_bus)
    , ws_server_(ws_server)
{
}

CapabilityManager::~CapabilityManager() {
    // Invalidate all capabilities
    for (auto& extensionCaps : granted_capabilities_) {
        for (auto& cap : extensionCaps) {
            if (auto locCap = std::dynamic_pointer_cast<LocationCapabilityImpl>(cap)) {
                locCap->invalidate();
            } else if (auto netCap = std::dynamic_pointer_cast<NetworkCapabilityImpl>(cap)) {
                netCap->invalidate();
            } else if (auto fsCap = std::dynamic_pointer_cast<FileSystemCapabilityImpl>(cap)) {
                fsCap->invalidate();
            } else if (auto uiCap = std::dynamic_pointer_cast<UICapabilityImpl>(cap)) {
                uiCap->invalidate();
            } else if (auto evCap = std::dynamic_pointer_cast<EventCapabilityImpl>(cap)) {
                evCap->invalidate();
            }
        }
    }
}

std::shared_ptr<capabilities::Capability> CapabilityManager::grantCapability(
    const QString& extensionId,
    const QString& capabilityType,
    const QVariantMap& options
) {
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

void CapabilityManager::revokeCapability(
    const QString& extensionId,
    const QString& capabilityType
) {
    QMutexLocker locker(&mutex_);
    
    if (granted_capabilities_.contains(extensionId) &&
        granted_capabilities_[extensionId].contains(capabilityType)) {
        
        auto cap = granted_capabilities_[extensionId][capabilityType];
        
        // Invalidate capability
        if (auto locCap = std::dynamic_pointer_cast<LocationCapabilityImpl>(cap)) {
            locCap->invalidate();
        } else if (auto netCap = std::dynamic_pointer_cast<NetworkCapabilityImpl>(cap)) {
            netCap->invalidate();
        } else if (auto fsCap = std::dynamic_pointer_cast<FileSystemCapabilityImpl>(cap)) {
            fsCap->invalidate();
        } else if (auto uiCap = std::dynamic_pointer_cast<UICapabilityImpl>(cap)) {
            uiCap->invalidate();
        } else if (auto evCap = std::dynamic_pointer_cast<EventCapabilityImpl>(cap)) {
            evCap->invalidate();
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
            if (auto locCap = std::dynamic_pointer_cast<LocationCapabilityImpl>(cap)) {
                locCap->invalidate();
            } else if (auto netCap = std::dynamic_pointer_cast<NetworkCapabilityImpl>(cap)) {
                netCap->invalidate();
            } else if (auto fsCap = std::dynamic_pointer_cast<FileSystemCapabilityImpl>(cap)) {
                fsCap->invalidate();
            } else if (auto uiCap = std::dynamic_pointer_cast<UICapabilityImpl>(cap)) {
                uiCap->invalidate();
            } else if (auto evCap = std::dynamic_pointer_cast<EventCapabilityImpl>(cap)) {
                evCap->invalidate();
            }
        }
        
        granted_capabilities_.remove(extensionId);
        
        logCapabilityUsage(extensionId, "all", "revoked_all", "");
        
        qInfo() << "Revoked all capabilities for:" << extensionId;
    }
}

bool CapabilityManager::hasCapability(
    const QString& extensionId,
    const QString& capabilityType
) const {
    QMutexLocker locker(&mutex_);
    
    return granted_capabilities_.contains(extensionId) &&
           granted_capabilities_[extensionId].contains(capabilityType) &&
           granted_capabilities_[extensionId][capabilityType]->isValid();
}

std::shared_ptr<capabilities::LocationCapability> CapabilityManager::getLocationCapability(const QString& extensionId) const {
    QMutexLocker locker(&mutex_);
    if (granted_capabilities_.contains(extensionId) && granted_capabilities_[extensionId].contains("location")) {
        return std::dynamic_pointer_cast<capabilities::LocationCapability>(granted_capabilities_[extensionId]["location"]);
    }
    return nullptr;
}

void CapabilityManager::logCapabilityUsage(
    const QString& extensionId,
    const QString& capabilityType,
    const QString& action,
    const QString& details
) {
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

QList<QVariantMap> CapabilityManager::getAuditLog(
    const QString& extensionId,
    int limit
) const {
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

bool CapabilityManager::shouldGrantPermission(
    const QString& extensionId,
    const QString& capabilityType,
    const QVariantMap& options
) const {
    // TODO: Check against manifest permissions
    // For now, grant all permissions (development mode)
    return true;
}

std::shared_ptr<capabilities::LocationCapability> CapabilityManager::createLocationCapability(
    const QString& extensionId,
    const QVariantMap& options
) {
    return std::make_shared<LocationCapabilityImpl>(extensionId, this);
}

std::shared_ptr<capabilities::NetworkCapability> CapabilityManager::createNetworkCapability(
    const QString& extensionId,
    const QVariantMap& options
) {
    return std::make_shared<NetworkCapabilityImpl>(extensionId, this);
}

std::shared_ptr<capabilities::FileSystemCapability> CapabilityManager::createFileSystemCapability(
    const QString& extensionId,
    const QVariantMap& options
) {
    // Determine scope path
    QString scopePath = options.value("scope_path").toString();
    
    if (scopePath.isEmpty()) {
        // Default: $CACHE/extensions/{extension-id}/
        scopePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
                    "/extensions/" + extensionId;
    }
    
    return std::make_shared<FileSystemCapabilityImpl>(extensionId, this, scopePath);
}

std::shared_ptr<capabilities::UICapability> CapabilityManager::createUICapability(
    const QString& extensionId,
    const QVariantMap& options
) {
    return std::make_shared<UICapabilityImpl>(extensionId, this);
}

std::shared_ptr<capabilities::EventCapability> CapabilityManager::createEventCapability(
    const QString& extensionId,
    const QVariantMap& options
) {
    return std::make_shared<EventCapabilityImpl>(extensionId, this, event_bus_);
}

std::shared_ptr<capabilities::Capability> CapabilityManager::createBluetoothCapability(
    const QString& extensionId,
    const QVariantMap& options
) {
    return std::static_pointer_cast<capabilities::Capability>(
        capabilities::createBluetoothCapabilityInstance(extensionId, this)
    );
}

}  // namespace core
}  // namespace openauto

// ===== CapabilityManager: UI registrar injection =====
namespace openauto {
namespace core {

void CapabilityManager::setUIRegistrar(ui::UIRegistrar* registrar) {
    QMutexLocker locker(&mutex_);
    ui_registrar_ = registrar;
}

} // namespace core
} // namespace openauto
