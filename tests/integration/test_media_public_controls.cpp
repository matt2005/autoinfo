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
#include <QVector>
#include <QPair>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>

#include "../../src/core/events/event_bus.hpp"
#include "../../src/core/capabilities/CapabilityManager.hpp"
#include "../../extensions/media_player/media_player_extension.hpp"

using namespace opencardev::crankshaft;

class TestMediaPublicControls : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Install a message handler to capture MediaPlayer logs
        qInstallMessageHandler(&TestMediaPublicControls::messageHandler);
    }

    void cleanupTestCase() {
        qInstallMessageHandler(nullptr);
    }

    void emits_play_is_received_by_media_player() {
        core::EventBus eventBus;
        core::CapabilityManager capManager(&eventBus, nullptr);

        // Prepare media player extension and grant event capability
        extensions::media::MediaPlayerExtension media;
        auto mediaEventCap = capManager.grantCapability("media_player", "event");
        QVERIFY(mediaEventCap != nullptr);
        media.grantCapability(mediaEventCap);
        QVERIFY(media.initialize());

        // Get a tester event capability and emit public media control event
        auto testerEv = std::dynamic_pointer_cast<core::capabilities::EventCapability>(
            capManager.grantCapability("tester", "event")
        );
        QVERIFY(testerEv != nullptr);

        // Clear previous logs
        {
            QMutexLocker lock(&s_logMutex);
            s_logs.clear();
        }

        // Emit public control event: "tester.media.play" (prefix added by emitEvent)
        QVariantMap payload; payload["from"] = "unit-test";
        QVERIFY(testerEv->emitEvent("media.play", payload));

        // Await the handler log
        bool seen = false;
        QElapsedTimer t; t.start();
        while (t.elapsed() < 500) {
            {
                QMutexLocker lock(&s_logMutex);
                for (const auto &msg : s_logs) {
                    if (msg.contains("Play command received")) { seen = true; break; }
                }
            }
            if (seen) break;
            QTest::qWait(10);
        }
        QVERIFY2(seen, "MediaPlayer did not receive public play command within timeout");

        media.stop();
        media.cleanup();
    }

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
        Q_UNUSED(type); Q_UNUSED(ctx);
        QMutexLocker lock(&s_logMutex);
        s_logs.append(msg);
    }

    static inline QVector<QString> s_logs;
    static inline QMutex s_logMutex;
};

QTEST_MAIN(TestMediaPublicControls)
#include "test_media_public_controls.moc"
