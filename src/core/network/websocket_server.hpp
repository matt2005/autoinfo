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

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include <QString>

namespace opencardev::crankshaft {
namespace core {

class WebSocketServer : public QObject {
    Q_OBJECT

public:
    explicit WebSocketServer(QObject *parent = nullptr);
    ~WebSocketServer() override;

    bool start(quint16 port);
    void stop();
    bool isRunning() const;
    
    void broadcast(const QString& message);
    void sendToClient(QWebSocket* client, const QString& message);

signals:
    void clientConnected(QWebSocket* client);
    void clientDisconnected(QWebSocket* client);
    void messageReceived(QWebSocket* client, const QString& message);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString& message);
    void onClientDisconnected();

private:
    QWebSocketServer* server_;
    QList<QWebSocket*> clients_;
};

}  // namespace core
}  // namespace opencardev::crankshaft
