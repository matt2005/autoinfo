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
#include <QVariantMap>
#include <QString>
#include <functional>
#include <QHash>
#include <QList>
#include <memory>

namespace opencardev::crankshaft {
namespace core {

using EventCallback = std::function<void(const QVariantMap&)>;

class EventBus : public QObject {
    Q_OBJECT

public:
    explicit EventBus(QObject *parent = nullptr);
    ~EventBus() override;

    // Subscribe to an event
    int subscribe(const QString& event_name, EventCallback callback);
    
    // Unsubscribe from an event
    void unsubscribe(int subscription_id);
    
    // Publish an event
    void publish(const QString& event_name, const QVariantMap& data = QVariantMap());

signals:
    void eventPublished(const QString& event_name, const QVariantMap& data);

private:
    struct Subscription {
        int id;
        QString event_name;
        EventCallback callback;
    };

    QHash<QString, QList<std::shared_ptr<Subscription>>> subscriptions_;
    int next_subscription_id_;
};

}  // namespace core
}  // namespace opencardev::crankshaft
