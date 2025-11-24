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

#include "websocket_server.hpp"
#include <QDebug>

namespace opencardev::crankshaft {
namespace core {

WebSocketServer::WebSocketServer(QObject* parent) : QObject(parent), server_(nullptr) {}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start(quint16 port) {
    if (server_) {
        qWarning() << "WebSocket server already running";
        return false;
    }

    server_ = new QWebSocketServer(QStringLiteral("Crankshaft Reborn WebSocket Server"),
                                   QWebSocketServer::NonSecureMode, this);

    if (server_->listen(QHostAddress::Any, port)) {
        qInfo() << "WebSocket server listening on port:" << port;
        connect(server_, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
        return true;
    } else {
        qWarning() << "Failed to start WebSocket server:" << server_->errorString();
        delete server_;
        server_ = nullptr;
        return false;
    }
}

void WebSocketServer::stop() {
    if (!server_) {
        return;
    }

    qInfo() << "Stopping WebSocket server...";

    for (QWebSocket* client : clients_) {
        client->close();
        client->deleteLater();
    }
    clients_.clear();

    server_->close();
    delete server_;
    server_ = nullptr;
}

bool WebSocketServer::isRunning() const {
    return server_ && server_->isListening();
}

void WebSocketServer::broadcast(const QString& message) {
    for (QWebSocket* client : clients_) {
        client->sendTextMessage(message);
    }
}

void WebSocketServer::sendToClient(QWebSocket* client, const QString& message) {
    if (clients_.contains(client)) {
        client->sendTextMessage(message);
    }
}

void WebSocketServer::onNewConnection() {
    QWebSocket* client = server_->nextPendingConnection();

    qInfo() << "New WebSocket client connected:" << client->peerAddress().toString();

    connect(client, &QWebSocket::textMessageReceived, this,
            &WebSocketServer::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &WebSocketServer::onClientDisconnected);

    clients_.append(client);
    emit clientConnected(client);
}

void WebSocketServer::onTextMessageReceived(const QString& message) {
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        qDebug() << "Message received from" << client->peerAddress().toString() << ":" << message;
        emit messageReceived(client, message);
    }
}

void WebSocketServer::onClientDisconnected() {
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        qInfo() << "Client disconnected:" << client->peerAddress().toString();
        clients_.removeAll(client);
        emit clientDisconnected(client);
        client->deleteLater();
    }
}

}  // namespace core
}  // namespace opencardev::crankshaft
