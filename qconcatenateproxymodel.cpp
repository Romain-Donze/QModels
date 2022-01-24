#include "qconcatenateproxymodel.h"

QConcatenateProxyModel::QConcatenateProxyModel(QObject *parent) :
    QAbstractListModel(parent)
{

}

QVariant QConcatenateProxyModel::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    int countTot=0;
    int count=0;
    int aColumn=-1;
    QString roleName="";

    if(role >= Qt::UserRole)
        aColumn = role-Qt::UserRole;
    else
        aColumn = role;

    roleName = QString::fromUtf8(roleNames().value(aColumn+Qt::UserRole));

    int localRoleIndex=-1;
    for(QAbstractItemModel* sourceModel: qAsConst(m_sourceModels))
    {
        countTot += sourceModel->rowCount();
        localRoleIndex=sourceModel->roleNames().key(roleName.toUtf8(),-1);
        if(index.row()<countTot)
            return sourceModel->data(sourceModel->index(index.row()-count,index.column()),localRoleIndex);
        count += sourceModel->rowCount();
    }

    return QVariant();
}
bool QConcatenateProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)

    return true;
}

int QConcatenateProxyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    int count=0;
    for(QAbstractItemModel* sourceModel: qAsConst(m_sourceModels))
    {
        count += sourceModel->rowCount();
    }

    return count;
}

int QConcatenateProxyModel::columnCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : roleNames().count();
}

QHash<int, QByteArray> QConcatenateProxyModel::roleNames() const
{
    if(m_sourceModels.isEmpty())
    {
        return QHash<int, QByteArray>();
    }
    return m_sourceModels.at(0)->roleNames();
}

void QConcatenateProxyModel::addSourceModel(QAbstractItemModel *sourceModel)
{
    if(sourceModel==nullptr)
        return;

    beginResetModel();
    if(m_sourceModels.size()==0)
    {
        connect(sourceModel, &QAbstractItemModel::columnsInserted, this, &QConcatenateProxyModel::columnsInserted);
        connect(sourceModel, &QAbstractItemModel::columnsRemoved, this, &QConcatenateProxyModel::columnsRemoved);
        connect(sourceModel, &QAbstractItemModel::layoutChanged, this, &QConcatenateProxyModel::layoutChanged);
     }
    connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &QConcatenateProxyModel::rowsInserted);
    connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, &QConcatenateProxyModel::rowsRemoved);
    connect(sourceModel, &QAbstractItemModel::modelReset, this, &QConcatenateProxyModel::modelReset);

    connect(sourceModel, &QAbstractItemModel::dataChanged, this, &QConcatenateProxyModel::onDataChanged);


    m_sourceModels.append(sourceModel);
    endResetModel();
}

void QConcatenateProxyModel::removeSourceModel(QAbstractItemModel *sourceModel)
{
    int index = m_sourceModels.indexOf(sourceModel);
    if(index < 0 || index >= rowCount())
        return;
    beginRemoveRows(QModelIndex(), index, index);
    m_sourceModels.at(index)->disconnect(this);
    m_sourceModels.removeAt(index);
    endRemoveRows();
}

void QConcatenateProxyModel::clearSourceModel()
{
    beginResetModel();
    m_sourceModels.clear();
    endResetModel();
}

void QConcatenateProxyModel::onDataChanged(const QModelIndex &from, const QModelIndex &to, const QVector<int> &roles)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(roles)

    beginResetModel();
    endResetModel();
}
