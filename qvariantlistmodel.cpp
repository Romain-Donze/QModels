#include "qvariantlistmodel.h"

// ──────── CONSTRUCTOR ──────────
QVariantListModel::QVariantListModel(QObject * parent) :
    QAbstractListModel(parent)
{
    QObject::connect(this, &QAbstractItemModel::rowsInserted, this, &QVariantListModel::countInvalidate);
    QObject::connect(this, &QAbstractItemModel::rowsRemoved, this, &QVariantListModel::countInvalidate);
    QObject::connect(this, &QAbstractItemModel::modelReset, this, &QVariantListModel::countInvalidate);
    QObject::connect(this, &QAbstractItemModel::layoutChanged, this, &QVariantListModel::countInvalidate);
}

const QModelIndex& QVariantListModel::noParent()
{
    static const QModelIndex ret = QModelIndex();
    return ret;
}

// ──────── ABSTRACT MODEL OVERRIDE ──────────
bool QVariantListModel::setData(const QModelIndex & index, const QVariant & data, const int role)
{
    bool ret=false;
    if (!index.parent ().isValid () &&
        index.column () == 0 &&
        index.row () >= 0 &&
        index.row () < m_variants.count () &&
        role == Qt::UserRole)
    {
        m_variants.replace(index.row(), data);
        Q_EMIT dataChanged (index, index, QVector<int>{ });
        ret = true;
    }
    return ret;
}

QVariant QVariantListModel::data(const QModelIndex & index, const int role) const
{
    QVariant ret;
    if (!index.parent().isValid() &&
        index.column() == 0 &&
        index.row() >= 0 &&
        index.row() < m_variants.count() &&
        role == Qt::UserRole)
    {
        ret.setValue (m_variants.at(index.row()));
    }
    return ret;
}

QHash<int, QByteArray> QVariantListModel::roleNames (void) const
{
    static const QHash<int, QByteArray> ret
    {
        { Qt::UserRole, QByteArrayLiteral ("modelData") },
    };
    return ret;
}

int QVariantListModel::rowCount (const QModelIndex & parent) const
{
    return (!parent.isValid() ? m_variants.count() : 0);
}

// ──────── PUBLIC API ──────────
QVariant QVariantListModel::at(const int index) const
{
    return get(index);
}

QVariant QVariantListModel::get(const int index) const
{
    if(index < 0 || index >= m_variants.size())
    {
        qWarning() << "The index" << index << "is out of bound.";
        return QVariant();
    }
    return m_variants.at(index);
}

bool QVariantListModel::append(const QVariant& variant)
{
    const int pos = m_variants.count();
    beginInsertRows(noParent(), pos, pos);
    m_variants.append(variant);
    endInsertRows();
    return true;
}

bool QVariantListModel::prepend(const QVariant& variant)
{
    beginInsertRows(noParent(), 0, 0);
    m_variants.prepend(variant);
    endInsertRows();
    return true;
}

bool QVariantListModel::insert(int index, const QVariant& variant)
{
    if(index > count())
    {
        qWarning() << "index " << index << " is greater than count " << count() << ". "
                   << "The item will be inserted at the end of the list";
        index = count();
    }
    else if(index < 0)
    {
        qWarning() << "index " << index << " is lower than 0. "
                   << "The item will be inserted at the beginning of the list";
        index = 0;
    }

    beginInsertRows(noParent(), index, index);
    m_variants.insert(index, variant);
    endInsertRows();
    return true;
}

bool QVariantListModel::replace (int index, const QVariant& variant)
{
    if(index > count())
    {
        qWarning() << "index " << index << " is greater than count " << count() << ". "
                   << "The item will be inserted at the end of the list";
        index = count();
    }
    else if(index < 0)
    {
        qWarning() << "index " << index << " is lower than 0. "
                   << "The item will be inserted at the beginning of the list";
        index = 0;
    }

    m_variants.replace(index, variant);
    Q_EMIT dataChanged(this->index(index), this->index(index), QVector<int> { });
    return true;
}

bool QVariantListModel::remove(int index, int count)
{
    if(index < 0 || (index + count - 1) >= m_variants.size())
    {
        qWarning() << "Can't remove an object whose index is out of bound";
        return false;
    }

    beginRemoveRows(noParent(), index, index + count - 1);
    for(int i = 0; i < count; ++i)
    {
        m_variants.removeAt(index);
    }
    endRemoveRows();

    return true;
}

bool QVariantListModel::clear()
{
    if(m_variants.isEmpty())
        return true;

    beginResetModel();
    m_variants.clear();
    endResetModel();

    return true;
}

// ──────── ABSTRACT MODEL PRIVATE ──────────
void QVariantListModel::countInvalidate()
{
    int aCount = count();
    bool aEmptyChanged=false;

    if(m_count==aCount)
        return;

    if((m_count==0 && aCount!=0) || (m_count!=0 && aCount==0))
        aEmptyChanged=true;

    m_count=aCount;
    Q_EMIT countChanged(count());

    if(aEmptyChanged)
        Q_EMIT emptyChanged(isEmpty());
}
