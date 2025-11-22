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
#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include "extensions/extension_manager.hpp"
#include "extensions/extension_manifest.hpp"

using namespace openauto::extensions;

class TestExtensionManager : public QObject {
    Q_OBJECT

private:
    QTemporaryDir tempDir;

    void createExtensionManifest(const QString& id, const QStringList& deps = QStringList()) {
        QString extPath = tempDir.path() + "/extensions/" + id;
        QDir().mkpath(extPath);
        
        QFile f(extPath + "/manifest.json");
        QVERIFY(f.open(QIODevice::WriteOnly));
        
        QString depsJson;
        for (int i = 0; i < deps.size(); ++i) {
            depsJson += QString("\"%1\"").arg(deps[i]);
            if (i < deps.size() - 1) depsJson += ",";
        }
        
        QString json = QString(
            "{\n"
            "  \"id\": \"%1\",\n"
            "  \"name\": \"%1 Test\",\n"
            "  \"version\": \"1.0.0\",\n"
            "  \"dependencies\": [%2],\n"
            "  \"requirements\": { \"required_permissions\": [] }\n"
            "}"
        ).arg(id, depsJson);
        
        f.write(json.toUtf8());
        f.close();
    }

private slots:
    void initTestCase() {
        QVERIFY(tempDir.isValid());
        QDir::setCurrent(tempDir.path());
        QDir().mkpath("extensions");
    }

    void test_load_simple_extension() {
        createExtensionManifest("simple_ext");
        
        ExtensionManager mgr;
        QSignalSpy loadedSpy(&mgr, &ExtensionManager::extensionLoaded);
        QSignalSpy errorSpy(&mgr, &ExtensionManager::extensionError);
        
        mgr.loadAll();
        
        QVERIFY(loadedSpy.count() >= 1);
        QVERIFY(errorSpy.count() == 0);
        
        bool found = false;
        for (const auto& args : loadedSpy) {
            if (args.at(0).toString() == "simple_ext") {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }
    
    void test_missing_dependency() {
        // Clean previous
        QDir extDir(tempDir.path() + "/extensions");
        extDir.removeRecursively();
        QDir().mkpath(tempDir.path() + "/extensions");
        
        createExtensionManifest("needs_missing", {"missing_dep"});
        
        ExtensionManager mgr;
        QSignalSpy errorSpy(&mgr, &ExtensionManager::extensionError);
        
        mgr.loadAll();
        
        QVERIFY(errorSpy.count() >= 1);
        bool foundMissingError = false;
        for (const auto& args : errorSpy) {
            if (args.at(0).toString() == "needs_missing" && 
                args.at(1).toString().contains("Missing dependencies")) {
                foundMissingError = true;
                break;
            }
        }
        QVERIFY(foundMissingError);
    }
    
    void test_valid_dependency_chain() {
        // Clean previous
        QDir extDir(tempDir.path() + "/extensions");
        extDir.removeRecursively();
        QDir().mkpath(tempDir.path() + "/extensions");
        
        createExtensionManifest("base_ext");
        createExtensionManifest("dep_ext", {"base_ext"});
        
        ExtensionManager mgr;
        QSignalSpy loadedSpy(&mgr, &ExtensionManager::extensionLoaded);
        QSignalSpy errorSpy(&mgr, &ExtensionManager::extensionError);
        
        mgr.loadAll();
        
        QCOMPARE(errorSpy.count(), 0);
        QVERIFY(loadedSpy.count() >= 2);
        
        QStringList loadedIds;
        for (const auto& args : loadedSpy) {
            loadedIds << args.at(0).toString();
        }
        
        QVERIFY(loadedIds.contains("base_ext"));
        QVERIFY(loadedIds.contains("dep_ext"));
        
        // Verify base_ext loads before dep_ext
        int baseIdx = loadedIds.indexOf("base_ext");
        int depIdx = loadedIds.indexOf("dep_ext");
        QVERIFY(baseIdx < depIdx);
    }
    
    void test_cycle_detection() {
        // Clean previous
        QDir extDir(tempDir.path() + "/extensions");
        extDir.removeRecursively();
        QDir().mkpath(tempDir.path() + "/extensions");
        
        createExtensionManifest("cycle_a", {"cycle_b"});
        createExtensionManifest("cycle_b", {"cycle_a"});
        
        ExtensionManager mgr;
        QSignalSpy errorSpy(&mgr, &ExtensionManager::extensionError);
        
        mgr.loadAll();
        
        QVERIFY(errorSpy.count() >= 2);
        
        int cycleErrors = 0;
        for (const auto& args : errorSpy) {
            QString error = args.at(1).toString().toLower();
            if (error.contains("cycle") || error.contains("circular")) {
                cycleErrors++;
            }
        }
        QVERIFY(cycleErrors >= 2);
    }
};

QTEST_MAIN(TestExtensionManager)
#include "test_extension_manager.moc"
