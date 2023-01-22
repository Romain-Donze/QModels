#include "qmodels_qmltypes.h"

#include "qobjectlistmodel.h"
#include "qobjectlistproperty.h"
#include "qvariantlistmodel.h"
#include "qcheckableproxymodel.h"
#include "qconcatenateproxymodel.h"
#include "qemptymodel.h"
#include "qmodelhelper.h"
#include "qmodelmatcher.h"
#include "qjsontreemodel.h"
#include "qjsonlistmodel.h"
#include "qcsvlistmodel.h"

#include <QtQml>

static void QModels_registerTypes()
{

    const int    maj = 1;
    const int    min = 0;

    qmlRegisterModule("Eco.Tier1.Models", 1, 0);

    qmlRegisterType<QEmptyModel>("Eco.Tier1.Models", maj, min, "EmptyModel");
    qmlRegisterType<QVariantListModel>("Eco.Tier1.Models", maj, min, "VariantListModel");
    qmlRegisterType<QCheckableProxyModel>("Eco.Tier1.Models", maj, min, "CheckableProxyModel");
    qmlRegisterType<QConcatenateProxyModel>("Eco.Tier1.Models", maj, min, "ConcatenateProxyModel");
    qmlRegisterType<QModelMatcher>("Eco.Tier1.Models", maj, min, "ModelMatcher");
    qmlRegisterType<QJsonTreeModel>("Eco.Tier1.Models", maj, min, "JsonTreeModel");
    qmlRegisterType<QJsonListModel>("Eco.Tier1.Models", maj, min, "JsonListModel");
    qRegisterMetaType<QJsonListModel::JsonFormat>("QJsonListModel::JsonFormat");
    qmlRegisterType<QCsvListModel>("Eco.Tier1.Models", maj, min, "CsvListModel");
    qmlRegisterUncreatableType<QObjectListModelBase>("Eco.Tier1.Models", maj, min, "ObjectListModel", "ObjectListModel is an abstract base class !");
    qmlRegisterUncreatableType<QModelHelper>("Eco.Tier1.Models", maj, min, "ModelHelper", "ModelHelper is only available via attached properties !");
    qmlRegisterUncreatableType<QQmlPropertyMap>("Eco.Tier1.Models", maj, min, "PropertyMap", "PropertyMap is an abstract base class !");
    qmlRegisterUncreatableType<QAbstractItemModel>("Eco.Tier1.Models", maj, min, "AbstractModel", "AbstractModel is an abstract base class !");

}

#ifdef QMODELS_SHARED
Q_COREAPP_STARTUP_FUNCTION(QModels_registerTypes);
#endif

void Eco::Tier1::QModels::registerComponents()
{
    ::QModels_registerTypes();
}
