#include "qcheckableproxymodel.h"

QCheckableProxyModel::QCheckableProxyModel(QObject *parent) :
    QIdentityProxyModel(parent),
    m_selection(new QItemSelectionModel(this))
{
    connect(m_selection, &QItemSelectionModel::selectionChanged, this, &QCheckableProxyModel::onSelectionChanged);
}

QVariant QCheckableProxyModel::data(const QModelIndex &index, int role) const
{
    if(!sourceModel())
        return QVariant();

    if (role == Qt::CheckStateRole)
    {
        if (index.column() != 0)
        {
            return QVariant();
        }
        if (!m_selection)
        {
            return Qt::Unchecked;
        }

        QVariant ret = m_selection->selection().contains(mapToSource(index)) ? Qt::Checked : Qt::Unchecked;
        return ret;
    }

    return QIdentityProxyModel::data(index, role);
}

bool QCheckableProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!sourceModel())
        return false;

    if (role == Qt::CheckStateRole)
    {
        if (index.column() != 0)
        {
            return false;
        }
        if (!m_selection)
        {
            return false;
        }

        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        const QModelIndex srcIndex = mapToSource(index);
        m_selection->select(QItemSelection(srcIndex, srcIndex), state == Qt::Checked ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
        emit this->dataChanged(index, index);
        return true;
    }

    return QIdentityProxyModel::setData(index, value, role);
}

Qt::ItemFlags QCheckableProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.column() != 0)
    {
        return QIdentityProxyModel::flags(index);
    }

    return QIdentityProxyModel::flags(index) | Qt::ItemIsUserCheckable;
}

QHash<int, QByteArray> QCheckableProxyModel::roleNames() const
{
    return m_roleNames;
}

void QCheckableProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (sourceModel && (sourceModel->roleNames().isEmpty())) { // workaround for when a model has no roles and roles are added when the model is populated
        // QTBUG-57971
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &QCheckableProxyModel::initRoles);
        connect(sourceModel, &QAbstractItemModel::modelReset, this, &QCheckableProxyModel::initRoles);
    }

    QIdentityProxyModel::setSourceModel(sourceModel);
    m_selection->setModel(sourceModel);
}

void QCheckableProxyModel::resetInternalData()
{
    QIdentityProxyModel::resetInternalData();
    updateRoleNames();
}

QList<int> QCheckableProxyModel::selectedRows() const
{
    QModelIndexList indexList = m_selection->selection().indexes();
    QList<int> ret;
    for(int i=0;i<indexList.size();i++)
    {
        ret+=indexList.at(i).row();
    }

    return ret;
}

bool QCheckableProxyModel::isChecked(int index) const
{
    return m_selection->isSelected(this->index(index,0));
}

void QCheckableProxyModel::clear()
{
    m_selection->clear();
}

void QCheckableProxyModel::toggle(int index)
{
    const QModelIndex srcIndex = mapToSource(this->index(index,0));
    Qt::CheckState oldState = (Qt::CheckState)data(srcIndex, Qt::CheckStateRole).toInt();

    setData(srcIndex, oldState == Qt::Checked ? QItemSelectionModel::Deselect : QItemSelectionModel::Select, Qt::CheckStateRole);
}

QItemSelectionModel *QCheckableProxyModel::getSelection() const
{
    return m_selection;
}

void QCheckableProxyModel::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    const auto lstSelected = mapSelectionFromSource(selected);
    for (const QItemSelectionRange &range : lstSelected)
    {
        emit this->dataChanged(range.topLeft(), range.bottomRight());
    }

    const auto lstDeselected = mapSelectionFromSource(deselected);
    for (const QItemSelectionRange &range : lstDeselected)
    {
        emit this->dataChanged(range.topLeft(), range.bottomRight());
    }
}

void QCheckableProxyModel::updateRoleNames()
{
    if(sourceModel() && !sourceModel()->roleNames().isEmpty())
    {
        QHash<int, QByteArray> roles = sourceModel()->roleNames();
        roles[Qt::CheckStateRole] = QByteArrayLiteral("checkState");
        m_roleNames = roles;
    }
}

void QCheckableProxyModel::initRoles()
{
    if(sourceModel() && !sourceModel()->roleNames().isEmpty())
    {
        disconnect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &QCheckableProxyModel::initRoles);
        disconnect(sourceModel(), &QAbstractItemModel::modelReset, this, &QCheckableProxyModel::initRoles);
        resetInternalData();
    }
}

