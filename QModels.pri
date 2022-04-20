QT += core

CONFIG += c++17

INCLUDEPATH += $$PWD

# TODO: add QSyncable feature https://github.com/benlau/qsyncable

HEADERS += \
    $$PWD/qconcatenateproxymodel.h \
    $$PWD/qemptymodel.h \
    $$PWD/qmodelhelper.h \
    $$PWD/qmodelqmltypes.h \
    $$PWD/qobjectlistmodel.h \
    $$PWD/qobjectlistmodelbase.h \
    $$PWD/qobjectlistproperty.h \
    $$PWD/qobjecttreemodel.h \
    $$PWD/qvariantlistmodel.h

SOURCES += \
    $$PWD/qconcatenateproxymodel.cpp \
    $$PWD/qmodelhelper.cpp \
    $$PWD/qmodelqmltypes.cpp \
    $$PWD/qobjecttreemodel.cpp \
    $$PWD/qvariantlistmodel.cpp

DISTFILES += \
    $$PWD/QModels \
    $$PWD/README.md \
    $$PWD/LICENSE
