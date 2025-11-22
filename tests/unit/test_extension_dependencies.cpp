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
#include <QFile>
#include <QDir>
#include <QSignalSpy>
#include "extensions/extension_manager.hpp"
#include "extensions/extension_manifest.hpp"

using namespace openauto::extensions;

class TestExtensionDependencies : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        // Create a temporary working directory structure
        tempRoot_ = QDir::currentPath() + "/test_ext_root";
        QDir().mkpath(tempRoot_);
        QDir::setCurrent(tempRoot_);
        QDir().mkpath("extensions");
    }

    void cleanupTestCase() {
        QDir dir(tempRoot_);
        dir.removeRecursively();
    }

    void createExtension(const QString& id, const QStringList& deps) {
        QString extPath = QString("extensions/%1").arg(id);
        QDir().mkpath(extPath);
        QFile f(extPath + "/manifest.json");
        QVERIFY(f.open(QIODevice::WriteOnly));
        QString depsJson;
        for (int i = 0; i < deps.size(); ++i) {
            depsJson += QString("\"%1\"").arg(deps[i]);
            if (i < deps.size() - 1) depsJson += ",";
        }
        QString json = QString("{\n  \"id\": \"%1\",\n  \"name\": \"%1 Test\",\n  \"version\": \"0.0.1\",\n  \"dependencies\": [%2],\n  \"requirements\": { \"required_permissions\": [] }\n}").arg(id, depsJson);
        f.write(json.toUtf8());
        f.close();
    }

    void test_missing_dependency() {
        ExtensionManager mgr; // capability manager not needed for dependency tests
        // One extension with missing dependency
        createExtension("ext_b", {"ext_c"});
        QSignalSpy errorSpy(&mgr, &ExtensionManager::extensionError);
        mgr.loadAll();
        QVERIFY(errorSpy.count() >= 1);
        bool found = false;
        for (const auto &args : errorSpy) {
            if (args.at(0).toString() == "ext_b" && args.at(1).toString().contains("Missing dependencies")) {
                found = true;
            }
        }
        QVERIFY(found);
    }

    void test_valid_dependency_order() {
        ExtensionManager mgr;
        // Clean previous extensions
        QDir("extensions").removeRecursively();
        QDir().mkpath("extensions");
        createExtension("ext_a", {});
        createExtension("ext_b", {"ext_a"});
        QSignalSpy loadSpy(&mgr, &ExtensionManager::extensionLoaded);
        QSignalSpy errorSpy(&mgr, &ExtensionManager::extensionError);
        mgr.loadAll();
        // Expect both loaded and no errors
        QStringList loadedIds;
        for (const auto &args : loadSpy) loadedIds << args.at(0).toString();
        QVERIFY(loadedIds.contains("ext_a"));
        QVERIFY(loadedIds.contains("ext_b"));
        QCOMPARE(errorSpy.count(), 0);
    }

    void test_cycle_detection() {
        ExtensionManager mgr;
        QDir("extensions").removeRecursively();
        QDir().mkpath("extensions");
        createExtension("ext_a", {"ext_b"});
        createExtension("ext_b", {"ext_a"});
        QSignalSpy errorSpy(&mgr, &ExtensionManager::extensionError);
        mgr.loadAll();
        // Expect cycle errors for both
        QVERIFY(errorSpy.count() >= 2);
        bool aCycle = false, bCycle = false;
        for (const auto &args : errorSpy) {
            if (args.at(0).toString() == "ext_a" && args.at(1).toString().contains("cycle")) aCycle = true;
            if (args.at(0).toString() == "ext_b" && args.at(1).toString().contains("cycle")) bCycle = true;
        }
        QVERIFY(aCycle && bCycle);
    }

private:
    QString tempRoot_;
};

QTEST_MAIN(TestExtensionDependencies)
#include "test_extension_dependencies.moc"