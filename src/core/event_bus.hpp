/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

namespace openauto {
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

    std::unordered_map<QString, std::vector<std::shared_ptr<Subscription>>> subscriptions_;
    int next_subscription_id_;
};

}  // namespace core
}  // namespace openauto
