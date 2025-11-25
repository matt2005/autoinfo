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

#include <QtTest/QtTest>
#include "core/capabilities/CapabilityManager.hpp"
#include "core/events/event_bus.hpp"

using namespace opencardev::crankshaft::core;
using namespace opencardev::crankshaft::core::capabilities;

class TestCapabilityManager : public QObject {
    Q_OBJECT

private slots:
    void grant_and_revoke_single_capability() {
        EventBus bus;
        CapabilityManager mgr(&bus, nullptr);

        auto cap = mgr.grantCapability("test_ext", "ui");
        QVERIFY(cap != nullptr);
        QVERIFY(mgr.hasCapability("test_ext", "ui"));

        mgr.revokeCapability("test_ext", "ui");
        QVERIFY(!mgr.hasCapability("test_ext", "ui"));
    }

    void grant_multiple_then_revoke_all() {
        EventBus bus;
        CapabilityManager mgr(&bus, nullptr);

        QVERIFY(mgr.grantCapability("extA", "event") != nullptr);
        QVERIFY(mgr.grantCapability("extA", "filesystem") != nullptr);
        QVERIFY(mgr.hasCapability("extA", "event"));
        QVERIFY(mgr.hasCapability("extA", "filesystem"));

        mgr.revokeAllCapabilities("extA");
        QVERIFY(!mgr.hasCapability("extA", "event"));
        QVERIFY(!mgr.hasCapability("extA", "filesystem"));
    }

    void token_capability_basic() {
        EventBus bus;
        CapabilityManager mgr(&bus, nullptr);

        auto contacts = mgr.grantCapability("phone_ui", "contacts");
        QVERIFY(contacts != nullptr);
        QCOMPARE(contacts->id(), QString("contacts"));
        QVERIFY(mgr.hasCapability("phone_ui", "contacts"));
    }
};

QTEST_MAIN(TestCapabilityManager)
#include "test_capability_manager.moc"
