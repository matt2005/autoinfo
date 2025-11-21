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
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>
#include <QObject>

namespace openauto {
namespace core {
namespace capabilities {

/**
 * Network capability for HTTP/HTTPS access.
 * 
 * Extensions with this capability can:
 * - Make HTTP GET/POST/PUT/DELETE requests
 * - Download files
 * - Access REST APIs
 * 
 * Extensions cannot directly access QNetworkAccessManager.
 * All requests are logged and can be rate-limited by core.
 */
class NetworkCapability : public Capability {
public:
    virtual ~NetworkCapability() = default;
    
    QString id() const override { return "network"; }
    
    /**
     * Perform HTTP GET request.
     * Returns QNetworkReply* that extension must manage (delete when done).
     * 
     * @param url URL to fetch
     * @return Network reply object (caller owns)
     */
    virtual QNetworkReply* get(const QUrl& url) = 0;
    
    /**
     * Perform HTTP POST request.
     * 
     * @param url URL to post to
     * @param data Data to send in request body
     * @return Network reply object (caller owns)
     */
    virtual QNetworkReply* post(const QUrl& url, const QByteArray& data) = 0;
    
    /**
     * Perform HTTP PUT request.
     * 
     * @param url URL to put to
     * @param data Data to send in request body
     * @return Network reply object (caller owns)
     */
    virtual QNetworkReply* put(const QUrl& url, const QByteArray& data) = 0;
    
    /**
     * Perform HTTP DELETE request.
     * 
     * @param url URL to delete
     * @return Network reply object (caller owns)
     */
    virtual QNetworkReply* deleteResource(const QUrl& url) = 0;
    
    /**
     * Download file to local path.
     * Uses capability's filesystem scope if fileSystemCapability provided.
     * 
     * @param url URL to download
     * @param localPath Local file path (within capability scope)
     * @return Network reply object for tracking progress (caller owns)
     */
    virtual QNetworkReply* downloadFile(const QUrl& url, const QString& localPath) = 0;
    
    /**
     * Check if network is available.
     */
    virtual bool isOnline() const = 0;

protected:
    NetworkCapability() = default;
};

}  // namespace capabilities
}  // namespace core
}  // namespace openauto
