QT += core

CONFIG += c++17

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/qmodels_qmltypes.h \
    $$PWD/qconcatenateproxymodel.h \
    $$PWD/qemptymodel.h \
    $$PWD/qmodelhelper.h \
    $$PWD/qobjectlistmodel.h \
    $$PWD/qobjectlistmodelbase.h \
    $$PWD/qobjectlistproperty.h \
    $$PWD/qvariantlistmodel.h \
    $$PWD/qmodelmatcher.h \
    $$PWD/qjsontreemodel.h \
    $$PWD/qjsonlistmodel.h \
    $$PWD/qcheckableproxymodel.h \
    $$PWD/qcsvlistmodel.h \
    $$PWD/qmodels_log.h

SOURCES += \
    $$PWD/qmodels_qmltypes.cpp \
    $$PWD/qconcatenateproxymodel.cpp \
    $$PWD/qmodelhelper.cpp \
    $$PWD/qvariantlistmodel.cpp \
    $$PWD/qmodelmatcher.cpp \
    $$PWD/qjsontreemodel.cpp \
    $$PWD/qjsonlistmodel.cpp \
    $$PWD/qcheckableproxymodel.cpp \
    $$PWD/qcsvlistmodel.cpp

DISTFILES += \
    $$PWD/QModels \
    $$PWD/README.md \
    $$PWD/LICENSE
