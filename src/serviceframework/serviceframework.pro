load(qt_module)

TARGET = QtServiceFramework
QPRO_PWD = $PWD

CONFIG += module

QT = core

DEFINES += QT_BUILD_SFW_LIB QT_MAKEDLL

load(qt_module_config)

sfwdebug {
    DEFINES += QT_SFW_IPC_DEBUG
    QT += network
}

jsondb {
    DEFINES += QT_NO_DBUS QT_ADDON_JSONDB_LIB
    QT += jsondb
}

include(ipc/ipc.pri)

PUBLIC_HEADERS += qservice.h \
    qservicemanager.h \
    qserviceplugininterface.h \
    qserviceinterfacedescriptor.h \
    qservicefilter.h \
    qremoteserviceregister.h \
    qserviceclientcredentials.h
PRIVATE_HEADERS += servicemetadata_p.h \
    qserviceinterfacedescriptor_p.h \
    dberror_p.h \
    qserviceclientcredentials_p.h \
    qservicedebuglog_p.h

SOURCES += servicemetadata.cpp \
    qservicemanager.cpp \
    qserviceplugininterface.cpp \
    qserviceinterfacedescriptor.cpp \
    qservicefilter.cpp \
    dberror.cpp \
    qremoteserviceregister.cpp \
    qserviceclientcredentials.cpp \
    qservicedebuglog.cpp \
    qserviceoperations.cpp \
    qservicereply.cpp \
    qservicerequest.cpp

jsondb {
    SOURCES += databasemanager_jsondb.cpp
    PRIVATE_HEADERS += databasemanager_jsondb_p.h
} else {
    QT += sql
    SOURCES += servicedatabase.cpp \
        databasemanager.cpp
    PRIVATE_HEADERS += servicedatabase_p.h \
        databasemanager_p.h \
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \
    qserviceoperations_p.h \
    qservicereply.h \
    qservicereply_p.h \
    qservicerequest_p.h
