#include "qmodelqmltypes.h"

#include "qobjectlistmodel.h"
#include "qobjectlistproperty.h"
#include "qvariantlistmodel.h"
#include "qconcatenateproxymodel.h"
#include "qemptymodel.h"
#include "qmodelhelper.h"

#include <QtQml>

static void QModels_registerTypes()
{
    const int    maj = 1;
    const int    min = 0;

    qmlRegisterModule("Models", 1, 0);

    qmlRegisterType<QEmptyModel>("Models", maj, min, "EmptyModel");
    qmlRegisterType<QVariantListModel>("Models", maj, min, "VariantListModel");
    qmlRegisterType<QConcatenateProxyModel>("Models", maj, min, "ConcatenateProxyModel");
    qmlRegisterUncreatableType<QModelHelper>("Models", maj, min, "ModelHelper", "ModelHelper is only available via attached properties !");
    qmlRegisterUncreatableType<QQmlPropertyMap>("Models", maj, min, "PropertyMap", "PropertyMap is an abstract base class !");
    qmlRegisterUncreatableType<QAbstractItemModel>("Models", maj, min, "AbstractModel", "AbstractModel is an abstract base class !");

}

#ifdef QMODELS_SHARED
Q_COREAPP_STARTUP_FUNCTION(QModels_registerTypes);
#endif

void QModels::registerComponents()
{
    ::QModels_registerTypes();
}
