#include "qmodelmatcher.h"

#include <QElapsedTimer>

QModelMatcher::QModelMatcher(QObject* parent):
    QObject(parent)
{
    connect(this, &QModelMatcher::sourceModelChanged, this, &QModelMatcher::updateRoles);

    QMetaObject::invokeMethod(this, &QModelMatcher::countInvalidate, Qt::QueuedConnection);
}

void QModelMatcher::classBegin()
{

}

void QModelMatcher::componentComplete()
{
    queueInvalidate();
}

void QModelMatcher::countInvalidate()
{
    const int aCount = count();
    bool emptyChanged=false;

    if(m_count==aCount)
        return;

    if((m_count==0 && aCount!=0) || (m_count!=0 && aCount==0))
        emptyChanged=true;

    m_count=aCount;

    emit this->countChanged();

    if(emptyChanged)
        emit this->emptyChanged();
}

void QModelMatcher::queueInvalidate()
{
    if (m_delayed) {
        if (!m_invalidateQueued) {
            m_invalidateQueued = true;
            QMetaObject::invokeMethod(this, &QModelMatcher::invalidate, Qt::QueuedConnection);
        }
    } else {
        invalidate();
    }
}

void QModelMatcher::invalidate()
{
    m_invalidateQueued = false;
    updateRoles();

    emit aboutToBeInvalidated();

    m_indexes.clear();

    if(m_sourceModel)
    {
        m_indexes = m_sourceModel->match(m_sourceModel->index(m_startRow, m_startColumn),
                                         m_role,
                                         m_value,
                                         m_hits,
                                         m_flags);
    }

    emit indexesChanged(m_indexes);
    countInvalidate();

    emit invalidated();
}

void QModelMatcher::updateFilterRole()
{
    if(!m_sourceModel)
        return;

    QList<int> filterRoles = m_sourceModel->roleNames().keys(m_roleName.toUtf8());
    if (!filterRoles.empty())
    {
        setRole(filterRoles.first());
    }
}

void QModelMatcher::updateRoles()
{
    updateFilterRole();
}

void QModelMatcher::initRoles()
{
    if(m_sourceModel && !m_sourceModel->roleNames().isEmpty())
    {
        disconnect(m_sourceModel, &QAbstractItemModel::rowsInserted, this, &QModelMatcher::initRoles);
        disconnect(m_sourceModel, &QAbstractItemModel::modelReset, this, &QModelMatcher::initRoles);
        updateRoles();
    }
}

const QModelIndexList &QModelMatcher::getIndexes() const
{
    return m_indexes;
}

bool QModelMatcher::getDelayed() const
{
    return m_delayed;
}

void QModelMatcher::setDelayed(bool delayed)
{
    if (m_delayed == delayed)
        return;
    m_delayed = delayed;
    emit delayedChanged(m_delayed);
}

QAbstractItemModel *QModelMatcher::getSourceModel() const
{
    return m_sourceModel;
}

void QModelMatcher::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (m_sourceModel == sourceModel)
        return;

    if(m_sourceModel)
        disconnect(m_sourceModel, nullptr, this, nullptr);

    m_sourceModel = sourceModel;

    if(m_sourceModel)
    {
        connect(m_sourceModel, &QAbstractItemModel::modelReset, this, &QModelMatcher::updateRoles);

        connect(m_sourceModel, &QAbstractItemModel::dataChanged, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::headerDataChanged, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::rowsInserted, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::columnsInserted, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::rowsRemoved, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::columnsRemoved, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::rowsMoved, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::columnsMoved, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::layoutChanged, this, &QModelMatcher::queueInvalidate);
        connect(m_sourceModel, &QAbstractItemModel::modelReset, this, &QModelMatcher::queueInvalidate);
    }

    emit sourceModelChanged(m_sourceModel);
}

int QModelMatcher::getStartRow() const
{
    return m_startRow;
}

void QModelMatcher::setStartRow(int startRow)
{
    if (m_startRow == startRow)
        return;
    m_startRow = startRow;
    queueInvalidate();
    emit startRowChanged(m_startRow);
}

int QModelMatcher::getStartColumn() const
{
    return m_startColumn;
}

void QModelMatcher::setStartColumn(int startColumn)
{
    if (m_startColumn == startColumn)
        return;
    m_startColumn = startColumn;
    queueInvalidate();
    emit startColumnChanged(m_startColumn);
}

int QModelMatcher::getRole() const
{
    return m_role;
}

void QModelMatcher::setRole(int role)
{
    if (m_role == role)
        return;
    m_role = role;
    queueInvalidate();
    emit roleChanged(m_role);
}

const QString &QModelMatcher::getRoleName() const
{
    return m_roleName;
}

void QModelMatcher::setRoleName(const QString &roleName)
{
    if (m_roleName == roleName)
        return;
    m_roleName = roleName;
    updateFilterRole();
    emit roleNameChanged(m_roleName);
}

const QVariant &QModelMatcher::getValue() const
{
    return m_value;
}

void QModelMatcher::setValue(const QVariant &value)
{
    if (m_value == value)
        return;
    m_value = value;
    queueInvalidate();
    emit valueChanged(m_value);
}

int QModelMatcher::getHits() const
{
    return m_hits;
}

void QModelMatcher::setHits(int hits)
{
    if (m_hits == hits)
        return;
    m_hits = hits;
    queueInvalidate();
    emit hitsChanged(m_hits);
}

const Qt::MatchFlags &QModelMatcher::getFlags() const
{
    return m_flags;
}

void QModelMatcher::setFlags(const Qt::MatchFlags &flags)
{
    if (m_flags == flags)
        return;
    m_flags = flags;
    queueInvalidate();
    emit flagsChanged(m_flags);
}
