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
#include <QRegularExpression>

namespace opencardev::crankshaft {
namespace core {

EventBus::EventBus(QObject *parent)
    : QObject(parent), next_subscription_id_(1) {
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
    
    subscriptions_[event_name].append(subscription);
    
    qDebug() << "Subscribed to event:" << event_name << "with ID:" << id;
    return id;
}

void EventBus::unsubscribe(int subscription_id) {
    for (auto it = subscriptions_.begin(); it != subscriptions_.end(); ++it) {
        auto &subs = it.value();
        bool removed = false;
        for (int i = subs.size() - 1; i >= 0; --i) {
            if (subs[i]->id == subscription_id) {
                subs.removeAt(i);
                removed = true;
                break;
            }
        }
        if (removed) {
            qDebug() << "Unsubscribed from event with ID:" << subscription_id;
            return;
        }
    }
}

void EventBus::publish(const QString& event_name, const QVariantMap& data) {
    qDebug() << "Publishing event:" << event_name;
    
    emit eventPublished(event_name, data);
    
    // First deliver exact-match subscriptions
    auto it = subscriptions_.find(event_name);
    if (it != subscriptions_.end()) {
        const auto &subs = it.value();
        for (const auto &subscription : subs) {
            subscription->callback(data);
        }
    }

    // Then deliver wildcard pattern subscriptions (e.g., "*.media.play", "navigation.*")
    auto matchesPattern = [](const QString& pattern, const QString& text) -> bool {
        // Fast path: no wildcard in pattern => no match handling here
        if (!pattern.contains('*') && !pattern.contains('?')) return false;
        // Escape regex special chars, then replace glob wildcards
        QString rx = QRegularExpression::escape(pattern);
        rx.replace("\\*", ".*");
        rx.replace("\\?", ".");
        QRegularExpression re("^" + rx + "$", QRegularExpression::UseUnicodePropertiesOption);
        return re.match(text).hasMatch();
    };

    for (auto it2 = subscriptions_.cbegin(); it2 != subscriptions_.cend(); ++it2) {
        const QString& pattern = it2.key();
        // Skip exact key already delivered
        if (pattern == event_name) continue;
        if (!matchesPattern(pattern, event_name)) continue;
        const auto &subs = it2.value();
        for (const auto &subscription : subs) {
            subscription->callback(data);
        }
    }
}

}  // namespace core
}  // namespace opencardev::crankshaft
