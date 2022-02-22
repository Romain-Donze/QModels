#ifndef QOBJECTTREEMODEL_H
#define QOBJECTTREEMODEL_H

#include "qobjectlistmodel.h"

//using QObjectTreeModel = QObjectListModel<class QObjectTreeElement>;

class QObjectTreeElement : public QObjectListModel<QObjectTreeElement>
{
    Q_OBJECT
public:
    QObjectTreeElement(QObject* parent = nullptr);
};

#endif // QOBJECTTREEMODEL_H
