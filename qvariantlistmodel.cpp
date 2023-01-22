#include "qvariantlistmodel.h"
#include "qmodels_log.h"

// ──────── CONSTRUCTOR ──────────
QVariantListModel::QVariantListModel(QObject * parent) :
    QAbstractListModel(parent),
    m_canWrite(true)
{
    QObject::connect(this, &QAbstractItemModel::rowsInserted, this, &QVariantListModel::countInvalidate);
    QObject::connect(this, &QAbstractItemModel::rowsRemoved, this, &QVariantListModel::countInvalidate);
    QObject::connect(this, &QAbstractItemModel::modelReset, this, &QVariantListModel::countInvalidate);
    QObject::connect(this, &QAbstractItemModel::layoutChanged, this, &QVariantListModel::countInvalidate);

    QObject::connect(this, &QAbstractItemModel::dataChanged, this, &QVariantListModel::contentInvalidate);
    QObject::connect(this, &QAbstractItemModel::rowsInserted, this, &QVariantListModel::contentInvalidate);
    QObject::connect(this, &QAbstractItemModel::rowsRemoved, this, &QVariantListModel::contentInvalidate);
    QObject::connect(this, &QAbstractItemModel::modelReset, this, &QVariantListModel::contentInvalidate);
    QObject::connect(this, &QAbstractItemModel::layoutChanged, this, &QVariantListModel::contentInvalidate);
}

const QModelIndex& QVariantListModel::noParent()
{
    static const QModelIndex ret = QModelIndex();
    return ret;
}

// ──────── ABSTRACT MODEL OVERRIDE ──────────
QVariant QVariantListModel::data(const QModelIndex & index, const int role) const
{
    if(!index.isValid() || index.row() >= m_storage.count())
        return QVariant();

    if(role == Qt::UserRole)
    {
        return m_storage.at(index.row());
    }
    else if(role > Qt::UserRole)
    {
        if(m_storage.at(index.row()).type() == QVariant::Map)
        {
            const QVariantMap& original = m_storage.at(index.row()).toMap();
            return original[m_roleNames.value(role)];
       }
    }

    return QVariant();
}

bool QVariantListModel::setData(const QModelIndex & index, const QVariant & data, const int role)
{
    if(!m_canWrite)
    {
        QMODELSLOG_WARNING()<<"This is a read only model";
        return false;
    }

    if(!index.isValid() || index.row() >= m_storage.count())
        return false;

    bool ret=false;
    if(role == Qt::UserRole)
    {
        m_storage.replace(index.row(), data);
        emit this->dataChanged (index, index, QVector<int>{});
        ret = true;
    }
    else if(role > Qt::UserRole)
    {
        if(m_storage.at(index.row()).type() == QVariant::Map)
        {
//            QVariantMap original = m_storage.at(index.row()).toMap();
//            original[m_roleNames.value(role)] = data;
//            m_storage.replace(index.row(), original);

            QVariantMap& original = *reinterpret_cast<QVariantMap*>(m_storage[index.row()].data());
            original[m_roleNames.value(role)] = data;
            emit this->dataChanged (index, index, QVector<int>{role});
            ret = true;
        }
    }

    if(!ret)
        QMODELSLOG_WARNING()<<"cannot setData to:"<<index<<role;

    return ret;
}

QHash<int, QByteArray> QVariantListModel::roleNames (void) const
{
    return m_roleNames;
}

int QVariantListModel::rowCount (const QModelIndex & parent) const
{
    return (!parent.isValid() ? m_storage.count() : 0);
}

// ──────── PUBLIC API ──────────

const QStringList& QVariantListModel::fields() const
{
    return m_fields;
}

bool QVariantListModel::setFields(const QStringList& fields)
{
    if(!m_roleNames.isEmpty() || fields.isEmpty())
        return false;

    QVariantMap map;
    for(const QString& field: fields)
        map.insert(field, QVariant());
    updateRoleNames(map);

    m_fields = fields;
    emit this->fieldsChanged(m_fields);

    return true;
}

bool QVariantListModel::append(const QJSValue& value)
{
    return insert(count(), value);
}

bool QVariantListModel::prepend(const QJSValue& value)
{
    return insert(0, value);
}

bool QVariantListModel::insert(int index, const QJSValue& value)
{
    return insert(index, value.toVariant());
}

bool QVariantListModel::append(const QVariant& variant)
{
    return insert(count(), variant);
}

bool QVariantListModel::prepend(const QVariant& variant)
{
    return insert(0, variant);
}

bool QVariantListModel::insert(int index, const QVariant& variant)
{
    if(index > count())
    {
        QMODELSLOG_WARNING() << "index " << index << " is greater than count " << count() << ". "
                   << "The item will be inserted at the end of the list";
        index = count();
    }
    else if(index < 0)
    {
        QMODELSLOG_WARNING() << "index " << index << " is lower than 0. "
                   << "The item will be inserted at the beginning of the list";
        index = 0;
    }

    if(variant.type() == QVariant::List)
    {
        const QVariantList variants = variant.toList();
        if(variants.isEmpty())
            return true;

        updateRoleNames(variants.first());

        beginInsertRows(noParent(), index, index + variants.count() - 1);
        m_storage.reserve(m_storage.count() + variants.count());
        int offset = 0;
        for(const QVariant& var: variants)
        {
            const int idx = index+offset;
            m_storage.insert(idx, var);
            offset++;
        }
        endInsertRows();
    }
    else
    {
        updateRoleNames(variant);

        beginInsertRows(noParent(), index, index);
        m_storage.insert(index, variant);
        endInsertRows();
    }

    return true;
}

QVariant QVariantListModel::at(const int index) const
{
    return get(index);
}

QVariant QVariantListModel::get(const int index) const
{
    if(index < 0 || index >= m_storage.size())
    {
        QMODELSLOG_WARNING() << "The index" << index << "is out of bound.";
        return QVariant();
    }
    return m_storage.at(index);
}

bool QVariantListModel::move(int from, int to, int count)
{
    if(from < 0 || (from + count - 1) >= m_storage.size())
    {
        QMODELSLOG_WARNING() << "Can't move an object whose index is out of bound";
        return false;
    }

    if(to < 0 || (to + count - 1) >= m_storage.size())
    {
        QMODELSLOG_WARNING() << "Can't move an object to a position that is out of bound";
        return false;
    }

    beginMoveRows(QModelIndex(), from, from + count - 1, QModelIndex(), to > from ? to + count : to);
    for(int i = 0; i < count; ++i)
    {
        m_storage.move(from, to);
    }
    endMoveRows();

    return true;
}

bool QVariantListModel::remove(int index, int count)
{
    if(index < 0 || (index + count - 1) >= m_storage.size())
    {
        QMODELSLOG_WARNING() << "Can't remove an object whose index is out of bound";
        return false;
    }

    beginRemoveRows(noParent(), index, index + count - 1);
    for(int i = 0; i < count; ++i)
    {
        m_storage.removeAt(index);
    }
    endRemoveRows();

    return true;
}

bool QVariantListModel::remove(QList<int> indexes)
{
    bool ret=true;

    std::sort(indexes.rbegin(), indexes.rend());

    for(int index: indexes)
    {
        if(!remove(index))
            ret = false;
    }
    return ret;
}

bool QVariantListModel::clear()
{
    if(m_storage.isEmpty())
        return true;

    beginResetModel();
    m_storage.clear();
    endResetModel();

    return true;
}

const QVariantList& QVariantListModel::storage() const
{
    return m_storage;
}

bool QVariantListModel::setStorage(const QVariant& storage)
{
    beginResetModel();
    m_storage.clear();

    if(storage.type() == QVariant::List)
    {
        m_storage = storage.toList();
    }
    else
    {
        m_storage.append(storage);
    }


    if(!m_storage.isEmpty())
        updateRoleNames(m_storage.first());

    endResetModel();

    return true;
}

bool QVariantListModel::setSource(QAbstractItemModel* model)
{
    if(!model)
        return false;

    if(QVariantListModel* varModel = qobject_cast<QVariantListModel*>(model))
        return setStorage(varModel->storage());

    beginResetModel();
    m_storage.clear();
    m_storage.reserve(model->rowCount());

    const QHash<int,QByteArray> names = model->roleNames();
    for(int i=0; i<model->rowCount(); ++i)
    {
        QVariantMap map;
        const QModelIndex modelIndex = model->index(i, 0);
        for (QHash<int, QByteArray>::const_iterator it = names.begin(); it != names.end(); ++it)
        {
            map.insert(it.value(), model->data(modelIndex, it.key()));
        }
        m_storage.append(map);
    }

    if(!m_storage.isEmpty())
        updateRoleNames(m_storage.first());

    endResetModel();

    return true;
}

// ──────── ABSTRACT MODEL PRIVATE ──────────
void QVariantListModel::countInvalidate()
{
    const int aCount = count();
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

void QVariantListModel::contentInvalidate()
{
    emit this->storageChanged(m_storage);
}

void QVariantListModel::updateRoleNames(const QVariant& var)
{
    if(m_roleNames.isEmpty())
    {
        QHash<int, QByteArray> roleNames;
        roleNames[Qt::UserRole] = QByteArrayLiteral("qtVariant");

        if(var.type() == QVariant::Map)
        {
            const QVariantMap& firstElement = var.toMap();
            QMapIterator<QString,QVariant> iter(firstElement);
            int role = Qt::UserRole+1;

            while (iter.hasNext())
            {
                iter.next();
                roleNames[role] = iter.key().toLocal8Bit();
                role++;
            }
        }

        m_roleNames = roleNames;
    }
}
