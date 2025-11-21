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

#include "event_bus.hpp"
#include <QDebug>

namespace openauto {
namespace core {

EventBus::EventBus(QObject *parent)
    : QObject(parent), next_subscription_id_(0) {
}

EventBus::~EventBus() {
    subscriptions_.clear();
}

int EventBus::subscribe(const QString& event_name, EventCallback callback) {
    int id = next_subscription_id_++;
    
    auto subscription = std::make_shared<Subscription>();
    subscription->id = id;
    subscription->event_name = event_name;
    subscription->callback = callback;
    
    subscriptions_[event_name].push_back(subscription);
    
    qDebug() << "Subscribed to event:" << event_name << "with ID:" << id;
    return id;
}

void EventBus::unsubscribe(int subscription_id) {
    for (auto& [event_name, subs] : subscriptions_) {
        auto it = std::remove_if(subs.begin(), subs.end(),
            [subscription_id](const std::shared_ptr<Subscription>& sub) {
                return sub->id == subscription_id;
            });
        
        if (it != subs.end()) {
            subs.erase(it, subs.end());
            qDebug() << "Unsubscribed from event with ID:" << subscription_id;
            return;
        }
    }
}

void EventBus::publish(const QString& event_name, const QVariantMap& data) {
    qDebug() << "Publishing event:" << event_name;
    
    emit eventPublished(event_name, data);
    
    auto it = subscriptions_.find(event_name);
    if (it != subscriptions_.end()) {
        for (const auto& subscription : it->second) {
            subscription->callback(data);
        }
    }
}

}  // namespace core
}  // namespace openauto
