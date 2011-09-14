/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qdebug.h>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <qjson/parser.h>
#include <qvaluespace.h>
#include "qvaluespace_p.h"
#include <qsignalspy.h>

#include <jsondblayer_p.h>

#include <private/jsondb-connection_p.h>

class TestJsonDbLayer : public QObject
{
    Q_OBJECT

public:
    TestJsonDbLayer();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testAddWatch();
    void testRemoveWatches();
    void testChildren();
    void testId();
    void testItem();
    void testLayerOptions();
    void testName();
    void testNotifyInterest();
    void testOrder();
    void testRemoveHandle();
    void testRemoveSubTree();
    void testRemoveValue();
    void testSetProperty();
    void testSetValue();
    void testStartup();
    void testSupportsInterestNotification();
    void testSync();
    void testValue();
    void testInstance();

private:
    JsonDbLayer *layer;
};

class TestJsonDbPath : public QObject
{
    Q_OBJECT

public:
    TestJsonDbPath();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testJsonDbPath();
    void testJsonDbPathString();
    void testJsonDbPathOther();

    void testGetPath();

    void testOperatorAssign();
    void testOperatorEqual();

    void testOperatorPlusString();
    void testOperatorPlusPath();

    void testGetIdentifier();
};

class TestJsonDbHandle : public QObject
{
    Q_OBJECT

    public:
        TestJsonDbHandle();

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();

        void init();
        void cleanup();

        void testJsonDbHandle();
        void testValue();
        void testSetValue();
        void testUnsetValue();
        void testSubscribe();
        void testUnsubscribe();
        void testChildren();
        void testRemoveSubTree();
};



void createJsonObjects(const QStringList &objects)
{
    QString object;

    QJson::Parser parser;
    bool ok;

    foreach (object, objects) {
        QVariantMap map = JsonDbConnection::makeCreateRequest(parser.parse(object.toAscii(), &ok));

        JsonDbConnection::instance()->sync(map).value<QVariantMap>();
    }
}

void cleanupJsonDb()
{
    QVariantMap map = JsonDbConnection::makeQueryRequest("[*]");
    map = JsonDbConnection::instance()->sync(map).value<QVariantMap>();

    QVariant object;

    foreach (object, map["data"].value<QVariantList>()) {
        QVariantMap delMap = object.value<QVariantMap>();

        if (delMap.contains("_type") && ((delMap["_type"] == "com.nokia.mp.settings.ApplicationSettings") || (delMap["_type"] == "com.nokia.mp.settings.SystemSettings"))) {
            delMap = JsonDbConnection::makeRemoveRequest(object);
            JsonDbConnection::instance()->sync(delMap);
        }
    }

    QTest::qWait(100);
}

bool exists(QString query)
{
    QVariantMap map = JsonDbConnection::makeQueryRequest(query);

    map = JsonDbConnection::instance()->sync(map).value<QVariantMap>();

    if (map["length"].value<int>() == 0)
        return false;

    return true;
}

QVariantMap getObject(const QString &identifier)
{
    QVariantMap map = JsonDbConnection::makeQueryRequest(QString("[?identifier=\"%1\"]").arg(identifier));
    QVariantMap result = JsonDbConnection::instance()->sync(map).value<QVariantMap>();

    if (!result.contains("length") || (result["length"].value<int>() != 1))
        return QVariantMap();

    return result["data"].value<QVariantList>()[0].value<QVariantMap>();;
}


TestJsonDbLayer::TestJsonDbLayer()
{

}

void TestJsonDbLayer::initTestCase()
{

}

void TestJsonDbLayer::cleanupTestCase()
{
    //cleanupJsonDb();
}

void TestJsonDbLayer::init()
{
    /*handler.stop();
    handler.reset();
    handler.start();

    QTest::qWait(100);*/

    layer = new JsonDbLayer();
}

void TestJsonDbLayer::cleanup()
{
    //delete layer;
    delete layer;

    cleanupJsonDb();
}

void TestJsonDbLayer::testAddWatch()
{
    // addWatch() is currently not implemented
    //QVERIFY2(false, "Not implemented!");
}

void TestJsonDbLayer::testRemoveWatches()
{
    // removeWatches() is currently not implemented
    //QVERIFY2(false, "Not implemented!");
}

void TestJsonDbLayer::testChildren()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testChildren.app1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testChildren.sys1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testChildren.sub.app2\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testChildren.sub.sys2\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.testChildren", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        //QSet<QString> children = handle.children();
        QSet<QString> children = layer->children(quintptr(&handle));
        QVERIFY2(children.count() == 3, "children() method failed!");
        QVERIFY2(children.contains("app1"), "children() method failed!");
        QVERIFY2(children.contains("sub"), "children() method failed!");
        QVERIFY2(children.contains("sys1"), "children() method failed!");
    }
    catch(...) { }
}

void TestJsonDbLayer::testId()
{
    QVERIFY2(layer->id() == QVALUESPACE_JSONDB_LAYER, "id() failed!");
}

void TestJsonDbLayer::testItem()
{
    JsonDbHandle* handle = reinterpret_cast<JsonDbHandle*>(layer->item(NULL, ""));
    QVERIFY2(handle != NULL, "item() failed!");
    QVERIFY2(handle->path.getPath() == "", "item() failed!");

    JsonDbHandle* handle2 = reinterpret_cast<JsonDbHandle*>(layer->item(NULL, "testItemLayer/b/c"));
    QVERIFY2(handle2 != NULL, "item() failed!");
    QVERIFY2(handle2->path.getPath() == "testItemLayer.b.c", "item() failed!");

    JsonDbHandle* handle3 = reinterpret_cast<JsonDbHandle*>(layer->item(quintptr(handle), ""));
    QVERIFY2(handle3 != NULL, "item() failed!");
    QVERIFY2(handle3->path.getPath() == "", "item() failed!");

    JsonDbHandle* handle4 = reinterpret_cast<JsonDbHandle*>(layer->item(quintptr(handle2), "d/e"));
    QVERIFY2(handle4 != NULL, "item() failed!");
    QVERIFY2(handle4->path.getPath() == "testItemLayer.b.c.d.e", "item() failed!");
}

void TestJsonDbLayer::testLayerOptions()
{
    QVERIFY2((layer->layerOptions() | QValueSpace::WritableLayer)
             && (layer->layerOptions() | QValueSpace::PermanentLayer), "Not implemented!");
}

void TestJsonDbLayer::testName()
{
    QVERIFY2(layer->name() == "JSON DB Layer", "JSON DB layer name");
}

void TestJsonDbLayer::testNotifyInterest()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testNotifyInterest.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testNotifyInterest
    JsonDbHandle handle(NULL, "com.testNotifyInterest", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    layer->notifyInterest(quintptr(&handle), true);

    try {
        QSignalSpy spy(layer, SIGNAL(valueChanged()));
        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);

        layer->notifyInterest(quintptr(&handle), false);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);
    } catch(...) {
        return;
    }
}

void TestJsonDbLayer::testOrder()
{
    QVERIFY2(layer->order() == 0x1000, "order() failed!");
}

void TestJsonDbLayer::testRemoveHandle()
{
    JsonDbHandle* handle = new JsonDbHandle(NULL, "", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QSignalSpy spy(handle, SIGNAL(destroyed()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    layer->removeHandle(quintptr(handle));

    QTest::qWait(100);
    QCOMPARE(spy.count(), 1);
}

void TestJsonDbLayer::testRemoveSubTree()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testRemoveSubTree\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "/com.testRemoveSubTree", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Deleting a subtree is not supported
        QVERIFY2(!layer->removeSubTree(NULL, quintptr(&handle)), "removeSubTree() failed!");
    }
    catch(...) { }
}

void TestJsonDbLayer::testRemoveValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testUnsetValue.app\", \"setting1\":1}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testUnsetValue.system\", \"setting2\":2}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.testUnsetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle), "setting1"), "removeValue() failed!");//handle.unsetValue("setting1"), "unsetValue()");

        // Settings objects may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle), ""), "removeValue() failed!");

        JsonDbHandle handle2(NULL, "com.testUnsetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle2), "setting2"), "removeValue() failed!");

        // Settings objects may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle2), ""), "removeValue() failed!");
    } catch(...) {}
}

void TestJsonDbLayer::testSetProperty()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testSetProperty.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL, "com.testSetProperty", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    layer->setProperty(quintptr(&handle), QAbstractValueSpaceLayer::Publish);

    try {
        QSignalSpy spy(layer, SIGNAL(handleChanged(quintptr)));
        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);

        layer->setProperty(quintptr(&handle), (QAbstractValueSpaceLayer::Properties)0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);
    } catch(...) {
        return;
    }
}

void TestJsonDbLayer::testSetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testSetValue.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testSetValue.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.testSetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(layer->setValue(NULL, quintptr(&handle), "setting1", 42), "setValue() failed!");

        QVariant value;
        QVERIFY2(handle.value("setting1", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        JsonDbHandle handle2(NULL, "com.testSetValue.app.setting1", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(handle2.value("", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        JsonDbHandle handle3(NULL, "com.testSetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(layer->setValue(NULL, quintptr(&handle3), "setting2", 42), "setValue() failed!");

        QVERIFY2(handle3.value("setting2", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        // Creating a new setting is not allowed
        QVERIFY2(!layer->setValue(NULL, quintptr(&handle3), "new_setting", 42), "setValue() failed!");

        // Changing a whole settings object is not allowed
        QVERIFY2(!layer->setValue(NULL, quintptr(&handle3), "", 42), "setValue() failed!");

        // Creating a new settings object is not allowed
        JsonDbHandle handle4(NULL, "com.testSetValue.system2", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(!layer->setValue(NULL, quintptr(&handle4), "", 42), "setValue() failed!");
    } catch(...) {}
}

void TestJsonDbLayer::testStartup()
{
    QVERIFY2(layer->startup(QAbstractValueSpaceLayer::Server) == true, "startup() failed!");
    QVERIFY2(layer->startup(QAbstractValueSpaceLayer::Client) == true, "startup() failed!");
}

void TestJsonDbLayer::testSupportsInterestNotification()
{
    QVERIFY2(layer->supportsInterestNotification(), "supportsInterestNotification() failed!");
}

void TestJsonDbLayer::testSync()
{
    // Nothing to test because the back-end isn't assynchronous
}

void TestJsonDbLayer::testValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testValueLayer.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testValueLayer.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com/testValueLayer/app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVariant value;

        QVERIFY2(layer->value(quintptr(&handle), "setting1", &value), "value() failed!");
        int intValue = value.value<int>();
        QVERIFY2(intValue == 1, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(layer->value(quintptr(&handle), &value), "value() failed!");
        QVariantMap map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.testValueLayer.app") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting1"] == "1"),
                 "value() failed!");

        JsonDbHandle handle2(NULL, "com/testValueLayer/system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(layer->value(quintptr(&handle2), "setting2", &value), "value() failed!");
        intValue = value.value<int>();
        QVERIFY2(intValue == 2, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(layer->value(quintptr(&handle2), &value), "value() failed!");
        map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.testValueLayer.system") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting2"] == "2"),
                 "value() failed!");

        QVERIFY2(!layer->value(quintptr(&handle), "testValueLayer", &value), "value() failed!");
    } catch(...) {}
}

void TestJsonDbLayer::testInstance()
{
    QVERIFY2(JsonDbLayer::instance() != NULL, "instance() failed!");
}



TestJsonDbPath::TestJsonDbPath()
{

}

void TestJsonDbPath::initTestCase()
{

}

void TestJsonDbPath::cleanupTestCase()
{

}

void TestJsonDbPath::testJsonDbPath()
{
    JsonDbPath path;
    QVERIFY2(path.getPath() == "", "Path is not an empty string!");
}

void TestJsonDbPath::testJsonDbPathString()
{
    JsonDbPath path1("/a/b/c");
    QVERIFY2(path1.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path2("a/b/c");
    QVERIFY2(path2.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path3("/a/b/c/");
    QVERIFY2(path3.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path4("a/b/c/");
    QVERIFY2(path4.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path5("a.b.c");
    QVERIFY2(path5.getPath() == "a.b.c", "String constructor failed!");
}

void TestJsonDbPath::testJsonDbPathOther()
{
    JsonDbPath path1("/a/b/c");
    JsonDbPath path11(path1);
    QVERIFY2(path11.getPath() == "a.b.c", "Path constructor failed!");

    JsonDbPath path2("a/b/c");
    JsonDbPath path22(path2);
    QVERIFY2(path22.getPath() == "a.b.c", "Path constructor failed!");

    JsonDbPath path3("/a/b/c/");
    JsonDbPath path33(path3);
    QVERIFY2(path33.getPath() == "a.b.c", "Path constructor failed!");

    JsonDbPath path4("a/b/c/");
    JsonDbPath path44(path4);
    QVERIFY2(path44.getPath() == "a.b.c", "Path constructor failed!");
}

void TestJsonDbPath::testGetPath()
{
    // Already tested in constructor test methods above
}

void TestJsonDbPath::testOperatorAssign()
{
    JsonDbPath path1;
    JsonDbPath path11 = path1;
    QVERIFY2(path11.getPath() == "", "Assignment operator failed!");

    JsonDbPath path2("/a/b/c");
    JsonDbPath path22 = path2;
    QVERIFY2(path22.getPath() == "a.b.c", "Assignment operator failed!");
}

void TestJsonDbPath::testOperatorEqual()
{
    JsonDbPath path1;
    JsonDbPath path11;
    QVERIFY2(path1 == path11, "Equal operator failed!");

    JsonDbPath path2("/a/b/c");
    JsonDbPath path22("/a/b/c");
    QVERIFY2(path2 == path22, "Equal operator failed!");

    JsonDbPath path3;
    JsonDbPath path33("/a/b/c");
    QVERIFY2(!(path3 == path33), "Equal operator failed!");

    JsonDbPath path4("/a/b");
    JsonDbPath path44("/a/b/c");
    QVERIFY2(!(path4 == path44), "Equal operator failed!");
}

void TestJsonDbPath::testOperatorPlusString()
{
    JsonDbPath path1;
    QVERIFY2((path1 + "").getPath() == "", "Plus string operator failed!");

    JsonDbPath path2;
    QVERIFY2((path2 + "/a/b/c").getPath() == "a.b.c", "Plus string operator failed!");

    JsonDbPath path3("/a/b/c");
    QVERIFY2((path3 + "").getPath() == "a.b.c", "Plus string operator failed!");

    JsonDbPath path4("/a/b/c");
    QVERIFY2((path4 + "d/e").getPath() == "a.b.c.d.e", "Plus string operator failed!");

    JsonDbPath path5("/a/b/c/");
    QVERIFY2((path5 + "d/e").getPath() == "a.b.c.d.e", "Plus string operator failed!");

    JsonDbPath path6("/a/b/c/");
    QVERIFY2((path6 + "/d/e").getPath() == "a.b.c.d.e", "Plus string operator failed!");
}

void TestJsonDbPath::testOperatorPlusPath()
{
    JsonDbPath path1;
    JsonDbPath path11;
    QVERIFY2((path1 + path11).getPath() == "", "Plus path operator failed!");

    JsonDbPath path2;
    JsonDbPath path22("/a/b/c");
    QVERIFY2((path2 + path22).getPath() == "a.b.c", "Plus path operator failed!");

    JsonDbPath path3("/a/b/c");
    JsonDbPath path33;
    QVERIFY2((path3 + path33).getPath() == "a.b.c", "Plus path operator failed!");

    JsonDbPath path4("/a/b/c");
    JsonDbPath path44("d/e");
    QVERIFY2((path4 + path44).getPath() == "a.b.c.d.e", "Plus path operator failed!");

    JsonDbPath path5("/a/b/c/");
    JsonDbPath path55("d/e");
    QVERIFY2((path5 + path55).getPath() == "a.b.c.d.e", "Plus path operator failed!");

    JsonDbPath path6("/a/b/c/");
    JsonDbPath path66("/d/e");
    QVERIFY2((path6 + path66).getPath() == "a.b.c.d.e", "Plus path operator failed!");
}

void TestJsonDbPath::testGetIdentifier()
{
    QString path = "com.nokia.mail.setting1";
    QStringList parts = JsonDbPath::getIdentifier(path);

    QVERIFY2(parts.count() == 2, "JsonDbPath::getIdentifier(path) failed!");
    QVERIFY2(parts[0] == "com.nokia.mail", "JsonDbPath::getIdentifier(path) failed!");
    QVERIFY2(parts[1] == "setting1", "JsonDbPath::getIdentifier(path) failed!");
}


TestJsonDbHandle::TestJsonDbHandle()
{
}

void TestJsonDbHandle::initTestCase()
{

}

void TestJsonDbHandle::cleanupTestCase()
{

}

void TestJsonDbHandle::init()
{

}

void TestJsonDbHandle::cleanup()
{
    cleanupJsonDb();
}

void TestJsonDbHandle::testJsonDbHandle()
{
    JsonDbHandle handle(NULL, "", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle.path.getPath() == "", "JsonDbHandle constructor failed!");

    JsonDbHandle handle1(NULL, "/testJsonDbHandle/b/c", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle1.path.getPath() == "testJsonDbHandle.b.c", "JsonDbHandle constructor failed!");

    JsonDbHandle handle2(&handle1, "/d/e", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle2.path.getPath() == "testJsonDbHandle.b.c.d.e", "JsonDbHandle constructor failed!");

    JsonDbHandle handle3(&handle1, "", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle3.path.getPath() == "testJsonDbHandle.b.c", "JsonDbHandle constructor failed!");
}

void TestJsonDbHandle::testValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testValueLayer.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testValueLayer.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com/testValueLayer/app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVariant value;

        QVERIFY2(handle.value("setting1", &value), "value() failed!");
        int intValue = value.value<int>();
        QVERIFY2(intValue == 1, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(handle.value("", &value), "value() failed!");
        QVariantMap map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.testValueLayer.app") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting1"] == "1"),
                 "value() failed!");

        JsonDbHandle handle2(NULL, "com/testValueLayer/system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(handle2.value("setting2", &value), "value() failed!");
        intValue = value.value<int>();
        QVERIFY2(intValue == 2, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(handle2.value("", &value), "value() failed!");
        map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.testValueLayer.system") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting2"] == "2"),
                 "value() failed!");

        QVERIFY2(!handle.value("testValueLayer", &value), "value() failed!");
    } catch(...) {}
}

void TestJsonDbHandle::testSetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testSetValue.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testSetValue.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.testSetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(handle.setValue("setting1", 42), "setValue() failed!");

        QVariant value;
        QVERIFY2(handle.value("setting1", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        JsonDbHandle handle2(NULL, "com.testSetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(handle2.setValue("setting2", 42), "setValue() failed!");

        QVERIFY2(handle2.value("setting2", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        // Creating a new setting is not allowed
        QVERIFY2(!handle2.setValue("new_setting", 42), "setValue() failed!");

        // Changing a whole settings object is not allowed
        QVERIFY2(!handle2.setValue("", 42), "setValue() failed!");

        // Use handle with setting path
        JsonDbHandle handle3(NULL, "com.testSetValue.app.setting1", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(handle3.setValue("", 123), "setValue() failed!");
        QVERIFY2(handle3.value("", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 123, "setValue() failed!");

        // Creating a new settings object is not allowed
        JsonDbHandle handle4(NULL, "com.testSetValue.system2", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(!handle4.setValue("", 42), "setValue() failed!");
    } catch(...) {}
}

void TestJsonDbHandle::testUnsetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testUnsetValue.app\", \"setting1\":1}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testUnsetValue.system\", \"setting2\":2}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.testUnsetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!handle.unsetValue("setting1"), "unsetValue()");

        // Settings objects may not be deleted
        QVERIFY2(!handle.unsetValue(""), "unsetValue()");

        JsonDbHandle handle2(NULL, "com.testUnsetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!handle2.unsetValue("setting2"), "unsetValue()");

        // Settings objects may not be deleted
        QVERIFY2(!handle2.unsetValue(""), "unsetValue()");
    } catch(...) {}
}

void TestJsonDbHandle::testSubscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testSubscribe.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL, "com.testSubscribe", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    handle.subscribe();

    try {
        QSignalSpy spy(&handle, SIGNAL(valueChanged()));
        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);
    } catch(...) {
        return;
    }
}

void TestJsonDbHandle::testUnsubscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testUnsubscribe.app\", \"setting1\":1}";

    JsonDbHandle handle(NULL, "com.testUnsubscribe", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QSignalSpy spy(&handle, SIGNAL(valueChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    handle.subscribe();

    QTest::qWait(100);

    handle.unsubscribe();

    createJsonObjects(objects);

    QTest::qWait(100);

    QCOMPARE(spy.count(), 0);
}

void TestJsonDbHandle::testChildren()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testChildren.app1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testChildren.sys1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testChildren.sub.app2\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mp.settings.SystemSettings\", \"identifier\":\"com.testChildren.sub.sys2\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.testChildren", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QSet<QString> children = handle.children();
        QVERIFY2(children.count() == 3, "children() method failed!");
        QVERIFY2(children.contains("app1"), "children() method failed!");
        QVERIFY2(children.contains("sub"), "children() method failed!");
        QVERIFY2(children.contains("sys1"), "children() method failed!");
    }
    catch(...) { }
}

void TestJsonDbHandle::testRemoveSubTree()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mp.settings.ApplicationSettings\", \"identifier\":\"com.testRemoveSubTree\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "/com.testRemoveSubTree", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Deleting a subtree is not supported
        QVERIFY2(!handle.removeSubTree(), "removeSubTree() failed!");
    }
    catch(...) { }
}

#include "tst_testpublishsubscribe.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestJsonDbPath pathTest;
    TestJsonDbHandle handleTest;
    TestJsonDbLayer layerTest;

    return  QTest::qExec(&pathTest, argc, argv) \
            & QTest::qExec(&handleTest, argc, argv) \
            & QTest::qExec(&layerTest, argc, argv);
}