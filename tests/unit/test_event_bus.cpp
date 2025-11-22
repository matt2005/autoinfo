/*
[]: #  * Project: Crankshaft
[]: #  * This file is part of Crankshaft project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  Crankshaft is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  Crankshaft is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtTest/QtTest>
#include "core/events/event_bus.hpp"

using namespace opencardev::crankshaft::core;

class TestEventBus : public QObject {
    Q_OBJECT

private slots:
    void test_publish_subscribe() {
        EventBus bus;
        bool called = false;
        QVariantMap receivedData;
        
        int subId = bus.subscribe("test.event", [&](const QVariantMap& data) {
            called = true;
            receivedData = data;
        });
        
        QVERIFY(subId >= 1);
        
        QVariantMap payload;
        payload["key"] = "value";
        payload["number"] = 42;
        
        bus.publish("test.event", payload);
        
        QVERIFY(called);
        QCOMPARE(receivedData["key"].toString(), QString("value"));
        QCOMPARE(receivedData["number"].toInt(), 42);
        
        bus.unsubscribe(subId);
    }
    
    void test_multiple_subscribers() {
        EventBus bus;
        int count1 = 0;
        int count2 = 0;
        
        bus.subscribe("multi.event", [&](const QVariantMap&) { count1++; });
        bus.subscribe("multi.event", [&](const QVariantMap&) { count2++; });
        
        bus.publish("multi.event", QVariantMap());
        
        QCOMPARE(count1, 1);
        QCOMPARE(count2, 1);
        
        bus.publish("multi.event", QVariantMap());
        
        QCOMPARE(count1, 2);
        QCOMPARE(count2, 2);
    }
    
    void test_unsubscribe() {
        EventBus bus;
        int count = 0;
        
        int subId = bus.subscribe("unsub.event", [&](const QVariantMap&) { count++; });
        
        bus.publish("unsub.event", QVariantMap());
        QCOMPARE(count, 1);
        
        bus.unsubscribe(subId);
        
        bus.publish("unsub.event", QVariantMap());
        QCOMPARE(count, 1); // Still 1, not incremented
    }
    
    void test_different_events() {
        EventBus bus;
        int count1 = 0;
        int count2 = 0;
        
        bus.subscribe("event1", [&](const QVariantMap&) { count1++; });
        bus.subscribe("event2", [&](const QVariantMap&) { count2++; });
        
        bus.publish("event1", QVariantMap());
        QCOMPARE(count1, 1);
        QCOMPARE(count2, 0);
        
        bus.publish("event2", QVariantMap());
        QCOMPARE(count1, 1);
        QCOMPARE(count2, 1);
    }
};

QTEST_MAIN(TestEventBus)
#include "test_event_bus.moc"
