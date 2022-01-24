
#include "qobjectlistmodel.h"
#include "qobjectlistproperty.h"
#include "qvariantlistmodel.h"
#include "qconcatenateproxymodel.h"
#include "qemptymodel.h"
#include "qmodelhelper.h"

#include <QtQml>

namespace QModel {

void QModelsRegisterComponents(){

    qmlRegisterModule("Models", 1, 0);

    qmlRegisterType<QEmptyModel>("Models", 1, 0, "EmptyModel");
    qmlRegisterType<QVariantListModel>("Models", 1, 0, "VariantListModel");
    qmlRegisterType<QConcatenateProxyModel>("Models", 1, 0, "ConcatenateProxyModel");
    qmlRegisterUncreatableType<QModelHelper>("Models", 1, 0, "ModelHelper", "ModelHelper is only available via attached properties !");
    qmlRegisterUncreatableType<QQmlPropertyMap>("Models", 1, 0, "PropertyMap", "PropertyMap is an abstract base class !");
    qmlRegisterUncreatableType<QAbstractItemModel>("Models", 1, 0, "AbstractModel", "AbstractModel is an abstract base class !");
}

Q_COREAPP_STARTUP_FUNCTION(QModelsRegisterComponents)

}
