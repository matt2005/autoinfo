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
#include "NetworkCapabilityImpl.hpp"
#include "CapabilityManager.hpp"
#include <QNetworkRequest>

using namespace opencardev::crankshaft::core::capabilities;
using opencardev::crankshaft::core::CapabilityManager;

NetworkCapabilityImpl::NetworkCapabilityImpl(const QString& extension_id, CapabilityManager* manager)
    : extension_id_(extension_id), manager_(manager), is_valid_(true), network_manager_(new QNetworkAccessManager()) {}

NetworkCapabilityImpl::~NetworkCapabilityImpl() { delete network_manager_; }

QString NetworkCapabilityImpl::extensionId() const { return extension_id_; }
bool NetworkCapabilityImpl::isValid() const { return is_valid_; }
void NetworkCapabilityImpl::invalidate() { is_valid_ = false; }

QNetworkReply* NetworkCapabilityImpl::get(const QUrl& url) {
    if (!is_valid_) return nullptr;
    manager_->logCapabilityUsage(extension_id_, "network", "get", url.toString());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "CrankshaftReborn/1.0");
    return network_manager_->get(request);
}

QNetworkReply* NetworkCapabilityImpl::post(const QUrl& url, const QByteArray& data) {
    if (!is_valid_) return nullptr;
    manager_->logCapabilityUsage(extension_id_, "network", "post", QString("%1 (%2 bytes)").arg(url.toString()).arg(data.size()));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "CrankshaftReborn/1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return network_manager_->post(request, data);
}

QNetworkReply* NetworkCapabilityImpl::put(const QUrl& url, const QByteArray& data) {
    if (!is_valid_) return nullptr;
    manager_->logCapabilityUsage(extension_id_, "network", "put", QString("%1 (%2 bytes)").arg(url.toString()).arg(data.size()));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "CrankshaftReborn/1.0");
    return network_manager_->put(request, data);
}

QNetworkReply* NetworkCapabilityImpl::deleteResource(const QUrl& url) {
    if (!is_valid_) return nullptr;
    manager_->logCapabilityUsage(extension_id_, "network", "delete", url.toString());
    QNetworkRequest request(url);
    return network_manager_->deleteResource(request);
}

QNetworkReply* NetworkCapabilityImpl::downloadFile(const QUrl& url, const QString& localPath) {
    if (!is_valid_) return nullptr;
    manager_->logCapabilityUsage(extension_id_, "network", "downloadFile", QString("%1 -> %2").arg(url.toString(), localPath));
    return get(url); // TODO implement proper download
}

bool NetworkCapabilityImpl::isOnline() const { return is_valid_ && network_manager_ != nullptr; }
