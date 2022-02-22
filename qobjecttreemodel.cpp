#include "qobjecttreemodel.h"

QObjectTreeElement::QObjectTreeElement(QObject* parent) :
    QObjectListModel<QObjectTreeElement>(parent)
{

}
