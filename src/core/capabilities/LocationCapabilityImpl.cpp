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
#include "LocationCapabilityImpl.hpp"
#include <QGeoPositionInfoSource>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "CapabilityManager.hpp"

using namespace opencardev::crankshaft::core::capabilities;
using opencardev::crankshaft::core::CapabilityManager;

LocationCapabilityImpl::LocationCapabilityImpl(const QString& extension_id,
                                               CapabilityManager* manager)
    : extension_id_(extension_id),
      manager_(manager),
      is_valid_(true),
      next_subscription_id_(1),
      position_source_(nullptr),
      device_mode_(DeviceMode::Internal),
      mock_timer_(nullptr) {}

LocationCapabilityImpl::~LocationCapabilityImpl() {
    if (position_source_) {
        position_source_->stopUpdates();
        delete position_source_;
    }
}

QString LocationCapabilityImpl::extensionId() const {
    return extension_id_;
}
bool LocationCapabilityImpl::isValid() const {
    return is_valid_;
}
void LocationCapabilityImpl::invalidate() {
    is_valid_ = false;
}

void LocationCapabilityImpl::ensurePositionSource() {
    if (!position_source_) {
        position_source_ = QGeoPositionInfoSource::createDefaultSource(nullptr);
        if (position_source_) {
            position_source_->startUpdates();
            QObject::connect(position_source_, &QGeoPositionInfoSource::positionUpdated,
                             [this](const QGeoPositionInfo& info) { onPositionUpdated(info); });
        }
    }
}

QGeoCoordinate LocationCapabilityImpl::getCurrentPosition() const {
    if (!is_valid_)
        return QGeoCoordinate();
    if (device_mode_ == DeviceMode::MockStatic || device_mode_ == DeviceMode::MockIP) {
        return mock_coordinate_;
    }
    const_cast<LocationCapabilityImpl*>(this)->ensurePositionSource();
    if (!position_source_)
        return QGeoCoordinate();
    manager_->logCapabilityUsage(extension_id_, "location", "getCurrentPosition");
    auto lastPos = position_source_->lastKnownPosition();
    return lastPos.coordinate();
}

int LocationCapabilityImpl::subscribeToUpdates(
    std::function<void(const QGeoCoordinate&)> callback) {
    if (!is_valid_)
        return -1;
    ensurePositionSource();
    int id = next_subscription_id_++;
    subscriptions_[id] = callback;
    manager_->logCapabilityUsage(extension_id_, "location", "subscribeToUpdates",
                                 QString("subscription_id=%1").arg(id));
    return id;
}

void LocationCapabilityImpl::unsubscribe(int subscriptionId) {
    subscriptions_.remove(subscriptionId);
    manager_->logCapabilityUsage(extension_id_, "location", "unsubscribe",
                                 QString("subscription_id=%1").arg(subscriptionId));
}

double LocationCapabilityImpl::getAccuracy() const {
    if (!is_valid_)
        return -1.0;
    if (device_mode_ == DeviceMode::MockStatic)
        return 25.0;
    if (device_mode_ == DeviceMode::MockIP)
        return 5000.0;
    if (!position_source_)
        return -1.0;
    auto lastPos = position_source_->lastKnownPosition();
    return lastPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)
               ? lastPos.attribute(QGeoPositionInfo::HorizontalAccuracy)
               : -1.0;
}

bool LocationCapabilityImpl::isAvailable() const {
    if (!is_valid_)
        return false;
    if (device_mode_ == DeviceMode::MockStatic || device_mode_ == DeviceMode::MockIP)
        return true;
    return position_source_ != nullptr;
}

void LocationCapabilityImpl::setDeviceMode(DeviceMode mode) {
    if (device_mode_ == mode)
        return;
    device_mode_ = mode;
    if (position_source_)
        position_source_->stopUpdates();
    if (mock_timer_)
        mock_timer_->stop();
    if (mode == DeviceMode::MockStatic) {
        mock_coordinate_ = QGeoCoordinate(51.5074, -0.1278);
        ensureMockTimer();
    } else if (mode == DeviceMode::MockIP) {
        QNetworkAccessManager* nm = new QNetworkAccessManager();
        QObject::connect(nm, &QNetworkAccessManager::finished, [this, nm](QNetworkReply* reply) {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonParseError parseError;
                QJsonDocument jd = QJsonDocument::fromJson(reply->readAll(), &parseError);
                if (parseError.error == QJsonParseError::NoError && jd.isObject()) {
                    QJsonObject obj = jd.object();
                    double lat = obj.value("lat").toDouble();
                    double lon = obj.value("lon").toDouble();
                    mock_coordinate_ = (lat != 0.0 || lon != 0.0)
                                           ? QGeoCoordinate(lat, lon)
                                           : QGeoCoordinate(51.5074, -0.1278);
                } else {
                    mock_coordinate_ = QGeoCoordinate(51.5074, -0.1278);
                }
            } else {
                mock_coordinate_ = QGeoCoordinate(51.5074, -0.1278);
            }
            reply->deleteLater();
            nm->deleteLater();
            ensureMockTimer();
        });
        nm->get(QNetworkRequest(QUrl("http://ip-api.com/json")));
    } else {
        ensurePositionSource();
        if (mock_timer_)
            mock_timer_->stop();
    }
}

LocationCapability::DeviceMode LocationCapabilityImpl::deviceMode() const {
    return device_mode_;
}

void LocationCapabilityImpl::onPositionUpdated(const QGeoPositionInfo& info) {
    if (!is_valid_)
        return;
    QGeoCoordinate coord = info.coordinate();
    for (const auto& cb : subscriptions_)
        cb(coord);
}

void LocationCapabilityImpl::ensureMockTimer() {
    if (!mock_timer_) {
        mock_timer_ = new QTimer();
        QObject::connect(mock_timer_, &QTimer::timeout, [this]() {
            if (device_mode_ == DeviceMode::MockStatic || device_mode_ == DeviceMode::MockIP) {
                for (const auto& cb : subscriptions_)
                    cb(mock_coordinate_);
            }
        });
    }
    mock_timer_->start(5000);
}
