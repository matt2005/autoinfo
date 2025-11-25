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

#include "NetworkCapability.hpp"
#include <QNetworkAccessManager>

namespace opencardev::crankshaft::core { class CapabilityManager; }

namespace opencardev::crankshaft::core::capabilities {

class NetworkCapabilityImpl : public NetworkCapability {
  public:
    NetworkCapabilityImpl(const QString& extension_id, core::CapabilityManager* manager);
    ~NetworkCapabilityImpl() override;

    QString extensionId() const override;
    bool isValid() const override;
    void invalidate();

    QNetworkReply* get(const QUrl& url) override;
    QNetworkReply* post(const QUrl& url, const QByteArray& data) override;
    QNetworkReply* put(const QUrl& url, const QByteArray& data) override;
    QNetworkReply* deleteResource(const QUrl& url) override;
    QNetworkReply* downloadFile(const QUrl& url, const QString& localPath) override;
    bool isOnline() const override;

  private:
    QString extension_id_;
    core::CapabilityManager* manager_;
    bool is_valid_;
    QNetworkAccessManager* network_manager_;
};

inline std::shared_ptr<NetworkCapability> createNetworkCapabilityInstance(const QString& extensionId, core::CapabilityManager* mgr) {
    return std::static_pointer_cast<NetworkCapability>(std::make_shared<NetworkCapabilityImpl>(extensionId, mgr));
}

} // namespace opencardev::crankshaft::core::capabilities
