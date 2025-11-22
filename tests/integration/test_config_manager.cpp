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
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QStandardPaths>

#include "core/config/ConfigManager.hpp"
#include "core/config/ConfigTypes.hpp"

using namespace opencardev::crankshaft::core::config;

class TestConfigManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Ensure predictable test environment
        qputenv("QT_HASH_SEED", QByteArray("1"));
    }

    void register_and_get_set_values() {
        ConfigManager mgr;

        ConfigItem itemBool;
        itemBool.key = "autoconnect";
        itemBool.label = "Auto connect";
        itemBool.type = ConfigItemType::Boolean;
        itemBool.defaultValue = true;

        ConfigItem itemInt;
        itemInt.key = "volume";
        itemInt.label = "Volume";
        itemInt.type = ConfigItemType::Integer;
        itemInt.defaultValue = 50;
        itemInt.properties["minValue"] = 0;
        itemInt.properties["maxValue"] = 100;

        ConfigItem itemSecret;
        itemSecret.key = "api_key";
        itemSecret.label = "API Key";
        itemSecret.type = ConfigItemType::String;
        itemSecret.defaultValue = "";
        itemSecret.isSecret = true;

        ConfigSection section;
        section.key = "general";
        section.title = "General";
        section.items = { itemBool, itemInt, itemSecret };

        ConfigPage page;
        page.domain = "core";
        page.extension = "test";
        page.title = "Test Page";
        page.sections = { section };

        mgr.registerConfigPage(page);
        // Ensure clean state independent of prior runs
        mgr.resetToDefaults("core","test");

        // Defaults
        QCOMPARE(mgr.getValue("core","test","general","autoconnect").toBool(), true);
        QCOMPARE(mgr.getValue("core","test","general","volume").toInt(), 50);

        // Set values
        QVERIFY(mgr.setValue("core","test","general","autoconnect", false));
        QVERIFY(mgr.setValue("core","test","general","volume", 80));
        QCOMPARE(mgr.getValue("core","test","general","autoconnect").toBool(), false);
        QCOMPARE(mgr.getValue("core","test","general","volume").toInt(), 80);
    }

    void export_import_masking_and_overwrite() {
        ConfigManager mgr;

        ConfigItem secret;
        secret.key = "token";
        secret.label = "Token";
        secret.type = ConfigItemType::String;
        secret.defaultValue = "default";
        secret.isSecret = true;

        ConfigSection sec;
        sec.key = "auth";
        sec.title = "Auth";
        sec.items = { secret };

        ConfigPage page;
        page.domain = "core";
        page.extension = "test2";
        page.title = "Test2";
        page.sections = { sec };

        mgr.registerConfigPage(page);
        QVERIFY(mgr.setValue("core","test2","auth","token", QString("SECRET-123")));

        // Export with masking
        QVariantMap masked = mgr.exportConfig(true);
        QVariantList pagesMasked = masked.value("pages").toList();
        QVariantMap pageMasked;
        for (const QVariant& v : pagesMasked) {
            QVariantMap p = v.toMap();
            if (p.value("domain").toString() == "core" && p.value("extension").toString() == "test2") {
                pageMasked = p; break;
            }
        }
        QVERIFY(!pageMasked.isEmpty());
        QVariantMap configMasked = pageMasked.value("config").toMap();
        QVariantMap auth = configMasked.value("auth").toMap();
        QVariant tokenMasked = auth.value("token");
        QVERIFY(tokenMasked.toString() != QString("SECRET-123"));

        // Export without masking and import overwrite behaviour
        QVariantMap unmasked = mgr.exportConfig(false);
        QVariantList pagesUnmasked = unmasked.value("pages").toList();
        QVariantMap pageUnmasked;
        for (const QVariant& v : pagesUnmasked) {
            QVariantMap p = v.toMap();
            if (p.value("domain").toString() == "core" && p.value("extension").toString() == "test2") {
                pageUnmasked = p; break;
            }
        }
        QVERIFY(!pageUnmasked.isEmpty());
        QVariantMap configUnmasked = pageUnmasked.value("config").toMap();
        QVariantMap unAuth = configUnmasked.value("auth").toMap();
        QCOMPARE(unAuth.value("token").toString(), QString("SECRET-123"));

        // Change value and then import with overwrite=false (should not override)
        QVERIFY(mgr.setValue("core","test2","auth","token", QString("LOCAL")));
        QVERIFY(mgr.importConfig(unmasked, /*overwriteExisting=*/false));
        QCOMPARE(mgr.getValue("core","test2","auth","token").toString(), QString("LOCAL"));

        // Now import with overwrite=true (should override)
        QVERIFY(mgr.importConfig(unmasked, /*overwriteExisting=*/true));
        QCOMPARE(mgr.getValue("core","test2","auth","token").toString(), QString("SECRET-123"));
    }

    void backup_and_restore_file_roundtrip() {
        ConfigManager mgr;

        ConfigItem item;
        item.key = "level";
        item.label = "Level";
        item.type = ConfigItemType::Integer;
        item.defaultValue = 1;

        ConfigSection sec;
        sec.key = "game";
        sec.title = "Game";
        sec.items = { item };

        ConfigPage page;
        page.domain = "core";
        page.extension = "backup";
        page.title = "Backup";
        page.sections = { sec };

        mgr.registerConfigPage(page);
        QVERIFY(mgr.setValue("core","backup","game","level", 42));

        QString tmpPath = QDir::temp().filePath("config_backup_test.json.gz");
        QFile::remove(tmpPath);

        QVERIFY2(mgr.backupToFile(tmpPath, /*maskSecrets=*/false, /*compress=*/true), "backupToFile failed");

        // Change value then restore
        QVERIFY(mgr.setValue("core","backup","game","level", 7));
        QVERIFY2(mgr.restoreFromFile(tmpPath, /*overwriteExisting=*/true), "restoreFromFile failed");
        QCOMPARE(mgr.getValue("core","backup","game","level").toInt(), 42);

        QFile::remove(tmpPath);
    }

    void complexity_level_set_get() {
        ConfigManager mgr;
        mgr.setComplexityLevel(ConfigComplexity::Advanced);
        QCOMPARE(mgr.getComplexityLevel(), ConfigComplexity::Advanced);
    }
};

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"
