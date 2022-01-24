#include "qmodelhelper.h"

#include <QSortFilterProxyModel>

//────────────────────────────────────────────────────────────────────────────────────────────────
// QModelHelperPropertyMap
//────────────────────────────────────────────────────────────────────────────────────────────────

class QModelHelperPropertyMap : public QQmlPropertyMap
{
public:
    QModelHelperPropertyMap(int row, int column, const QModelIndex& parentIndex, QAbstractItemModel* model, QObject* parent = nullptr) :
        QQmlPropertyMap(parent),
        m_row(row),
        m_column(column),
        m_parent(parentIndex),
        m_model(model)
    {
        connect(model, &QAbstractItemModel::modelReset, this, &QModelHelperPropertyMap::update);
        connect(model, &QAbstractItemModel::layoutChanged, this, &QModelHelperPropertyMap::update);
        connect(model, &QAbstractItemModel::dataChanged, this, &QModelHelperPropertyMap::onDataChanged);
        connect(model, &QAbstractItemModel::rowsInserted, this, &QModelHelperPropertyMap::onRowsInserted);
        connect(model, &QAbstractItemModel::rowsRemoved, this, &QModelHelperPropertyMap::onRowsRemoved);
        connect(model, &QAbstractItemModel::columnsInserted, this, &QModelHelperPropertyMap::onColumnsInserted);
        connect(model, &QAbstractItemModel::columnsInserted, this, &QModelHelperPropertyMap::onColumnsRemoved);
        update();
    }

protected:
    QVariant updateValue(const QString& key, const QVariant& input) override
    {
        int role = m_model->roleNames().key(key.toUtf8(), -1);
        if (role == -1)
            return input;

        QModelIndex index = modelIndex();
        m_model->setData(index, input, role);
        return m_model->data(index, role);
    }

private:
    QModelIndex modelIndex() const
    {
        return m_model->index(m_row, m_column, m_parent);
    }

    void update()
    {
        QHash<int, QByteArray> roles = m_model->roleNames();
        QModelIndex index(modelIndex());

        for (auto it = roles.cbegin(); it != roles.cend(); ++it)
        {
            insert(it.value(), m_model->data(index, it.key()));
        }
    }
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
    {
        QModelIndex index(modelIndex());
        if (m_parent != topLeft.parent() || m_parent != bottomRight.parent())
            return;

        if (m_row >= topLeft.row() && m_column >= topLeft.column() && m_row <= bottomRight.row() && m_column <= bottomRight.column()) {
            auto roleNames = m_model->roleNames();
            QVector<int> actualRoles;
            if(roles.isEmpty())
            {
                QList<int> keys = roleNames.keys();
                actualRoles = keys.toVector();
            }
            else
            {
                actualRoles = roles;
            }

            for (int role : qAsConst(actualRoles))
                insert(roleNames[role], m_model->data(index, role));
        }
    }
    void onRowsInserted(const QModelIndex& parent, int first, int last)
    {
        Q_UNUSED(last)
        if (parent == m_parent && m_row >= first)
            update();
    }
    void onRowsRemoved(const QModelIndex& parent, int first, int last)
    {
        if (parent == m_parent && m_row >= first && m_row<=last)
            update();
    }
    void onColumnsInserted(const QModelIndex& parent, int first, int last)
    {
        Q_UNUSED(last)
        if (parent == m_parent && m_column >= first)
            update();
    }
    void onColumnsRemoved(const QModelIndex& parent, int first, int last)
    {
        Q_UNUSED(last)
        if (parent == m_parent && m_column >= first)
            update();
    }

    int m_row;
    int m_column;
    QPersistentModelIndex m_parent;
    QAbstractItemModel* m_model;
};

//────────────────────────────────────────────────────────────────────────────────────────────────
// QModelHelperFilter
//────────────────────────────────────────────────────────────────────────────────────────────────

class QModelHelperFilter : public QSortFilterProxyModel
{
public:
   QModelHelperFilter(QObject *parent) :
       QSortFilterProxyModel(parent)
   {

   }

   void setFilterRoleName(const QString& filterRoleName)
   {
       if(filterRoleName==m_filterRoleName)
           return;
       m_filterRoleName=filterRoleName;
       QList<int> filterRoles = roleNames().keys(m_filterRoleName.toUtf8());
       if (!filterRoles.empty())
       {
           setFilterRole(filterRoles.first());
       }
   }
   void setFilterValue(const QVariant& filterValue)
   {
       if(filterValue==m_filterValue)
           return;
       m_filterValue=filterValue;
       invalidateFilter();
   }
   int filteredToSource(int proxyRow) const
   {
       QModelIndex proxyIndex = index(proxyRow, 0);
       QModelIndex sourceIndex = mapToSource(proxyIndex);
       return sourceIndex.isValid() ? sourceIndex.row() : -1;
   }

protected:
   bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override final
   {
       QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
       bool valueAccepted = !m_filterValue.isValid() || ( m_filterValue == sourceModel()->data(sourceIndex, filterRole()) );
       return valueAccepted;
   }

private:
   QString m_filterRoleName;
   QVariant m_filterValue;
};

//────────────────────────────────────────────────────────────────────────────────────────────────
// QModelHelper
//────────────────────────────────────────────────────────────────────────────────────────────────

// ──────── CONSTRUCTOR ──────────
QModelHelper::QModelHelper(QAbstractItemModel* object) :
    QAbstractItemModel(object),
    m_sourceModel(object),
    m_proxyModelPrivate(new QModelHelperFilter(object))
{
    if (!object || !m_sourceModel)
        qFatal("ModelHelper must be attached to a QAbstractItemModel*");
    else
    {
        m_proxyModelPrivate->setSourceModel(m_sourceModel);

        connect(m_sourceModel, &QAbstractItemModel::dataChanged, this, &QAbstractItemModel::dataChanged);
        connect(m_sourceModel, &QAbstractItemModel::headerDataChanged, this, &QAbstractItemModel::headerDataChanged);
        connect(m_sourceModel, &QAbstractItemModel::rowsInserted, this, &QAbstractItemModel::rowsInserted);
        connect(m_sourceModel, &QAbstractItemModel::rowsRemoved, this, &QAbstractItemModel::rowsRemoved);
        connect(m_sourceModel, &QAbstractItemModel::rowsMoved, this, &QAbstractItemModel::rowsMoved);
        connect(m_sourceModel, &QAbstractItemModel::columnsRemoved, this, &QAbstractItemModel::columnsRemoved);
        connect(m_sourceModel, &QAbstractItemModel::columnsInserted, this, &QAbstractItemModel::columnsInserted);
        connect(m_sourceModel, &QAbstractItemModel::columnsMoved, this, &QAbstractItemModel::columnsMoved);
        connect(m_sourceModel, &QAbstractItemModel::modelReset, this, &QAbstractItemModel::modelReset);
        connect(m_sourceModel, &QAbstractItemModel::layoutChanged, this, &QAbstractItemModel::layoutChanged);

        QObject::connect(this, &QAbstractItemModel::rowsInserted, this, &QModelHelper::countInvalidate);
        QObject::connect(this, &QAbstractItemModel::rowsRemoved, this, &QModelHelper::countInvalidate);
        QObject::connect(this, &QAbstractItemModel::modelReset, this, &QModelHelper::countInvalidate);
        QObject::connect(this, &QAbstractItemModel::layoutChanged, this, &QModelHelper::countInvalidate);

        QMetaObject::invokeMethod(this, &QModelHelper::countInvalidate, Qt::QueuedConnection);
    }
}

QModelHelper* QModelHelper::wrap(QObject* object)
{
    QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>(object);
    if (!model)
    {
        qFatal("ModelHelper must be attached to a QAbstractItemModel*");
        return nullptr;
    }

    QModelHelper* helper = model->findChild<QModelHelper*>(QString(), Qt::FindDirectChildrenOnly);
    if(!helper)
        helper = new QModelHelper(model);

    return helper;
}

QModelHelper* QModelHelper::qmlAttachedProperties(QObject* object)
{
    return wrap(object);
}

// ──────── ABSTRACT MODEL OVERRIDE ──────────
bool QModelHelper::setData(const QModelIndex& modelIndex, const QVariant& value, int role)
{
    return m_sourceModel->setData(modelIndex, value, role);
}
QVariant QModelHelper::data(const QModelIndex& modelIndex, int role) const
{
    return m_sourceModel->data(modelIndex, role);
}

QModelIndex QModelHelper::index(int row, int column, const QModelIndex &parent) const
{
    return m_sourceModel->index(row, column, parent);
}
QModelIndex QModelHelper::parent(const QModelIndex &child) const
{
    return m_sourceModel->parent(child);
}

QHash<int, QByteArray> QModelHelper::roleNames() const
{
    return m_sourceModel->roleNames();
}
int QModelHelper::rowCount(const QModelIndex &parent) const
{
    return m_sourceModel->rowCount(parent);
}
int QModelHelper::columnCount(const QModelIndex &parent) const
{
    return m_sourceModel->columnCount(parent);
}

int QModelHelper::roleForName(const QByteArray& name) const
{
    return roleNames().key(name, -1);
}
QByteArray QModelHelper::roleName(int role) const
{
    return roleNames().value(role,"");
}

// ──────── ABSTRACT MODEL PRIVATE ──────────
void QModelHelper::countInvalidate()
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

// ──────── PUBLIC API ──────────
QQmlPropertyMap* QModelHelper::map(int row, int column, const QModelIndex& parent)
{
    if (!m_sourceModel)
        return nullptr;

    if (column == 0 && !parent.isValid()) {
        QQmlPropertyMap* mapper = mapperForRow(row);
        if (!mapper) {
            mapper = new QModelHelperPropertyMap(row, 0, {}, m_sourceModel, this);
            m_mappers.append({row, mapper});
            QObject::connect(mapper, &QObject::destroyed, this, [=]{removeMapper(mapper);});
        }
        return mapper;
    }
    return new QModelHelperPropertyMap(row, column, parent, m_sourceModel, this);
}

bool QModelHelper::updateWhere(const QString& columnName, const QVariant& where, const QString& property, const QVariant& value)
{
    bool aRet = false;

    if(where.canConvert(QMetaType::QVariantList))
    {
        const QVariantList values = where.toList();

        if(values.count()>0)
        {
            int roleIndex = roleForName(property.toUtf8());
            m_proxyModelPrivate->setFilterRoleName(columnName);
            for(const QVariant& e : values)
            {
                m_proxyModelPrivate->setFilterValue(e);

                for(int i=m_proxyModelPrivate->rowCount()-1;i>=0;i--)
                {
                    aRet = m_proxyModelPrivate->setData(m_proxyModelPrivate->index(i, 0), value, roleIndex);
                }
            }
        }
    }
    else if(where.canConvert(QMetaType::QString))
    {
        if(where.toString()!="0")
        {
            int roleIndex = roleForName(property.toUtf8());
            m_proxyModelPrivate->setFilterRoleName(columnName);
            m_proxyModelPrivate->setFilterValue(where);

            for(int i=m_proxyModelPrivate->rowCount()-1;i>=0;i--)
            {
                aRet = m_proxyModelPrivate->setData(m_proxyModelPrivate->index(i, 0), value, roleIndex);
            }
        }
    }
    else
    {
        qDebug()<<"error";
    }

    return aRet;
}

bool QModelHelper::updateAll(const QString& property, const QVariant& value)
{
    bool aRet = false;

    int roleIndex = roleForName(property.toUtf8());
    for(int i=m_sourceModel->rowCount()-1;i>=0;i--)
    {
        aRet = m_sourceModel->setData(m_sourceModel->index(i, 0), value, roleIndex);
    }

    return aRet;
}

bool QModelHelper::set(int row, const QVariantMap& pArray)
{
    bool aRet=false;
    if (row >= count() || row < 0)
        return aRet;
    const QList<QString> keys = pArray.keys();
    for(const QString& key : keys)
    {
        aRet = setProperty(row,key,pArray.value(key));
    }

    return aRet;
}

bool QModelHelper::setProperty(int pIndex, const QString& property, const QVariant& value)
{
    int roleIndex = roleForName(property.toUtf8());

    if(roleIndex<0)
        return false;

    return setData(index(pIndex, 0), value, roleIndex);
}

QVariantMap QModelHelper::get(int row, const QStringList roles) const
{
    QHash<int,QByteArray> names = roleNames();
    if(!roles.isEmpty())
    {
        QHash<int,QByteArray> tmpNames=names;
        names.clear();
        for(const QString& role: roles)
            names[tmpNames.key(role.toUtf8())]=role.toUtf8();
    }

    QVariantMap map;
    QModelIndex modelIndex = index(row, 0);
    for (QHash<int, QByteArray>::iterator it = names.begin(); it != names.end(); ++it)
    {
        map.insert(it.value(), data(modelIndex, it.key()));
    }

    return map;
}

QVariant QModelHelper::getProperty(int pIndex, const QString& property) const
{
    int roleIndex = roleForName(property.toUtf8());

    if(roleIndex<0)
        return QVariant();

    return data(index(pIndex, 0), roleIndex);
}

int QModelHelper::indexOf(const QString &columnName, const QVariant &val) const
{
    m_proxyModelPrivate->setFilterRoleName(columnName);
    m_proxyModelPrivate->setFilterValue(val);

    return m_proxyModelPrivate->filteredToSource(0);
}

bool QModelHelper::contains(const QString &columnName, const QVariant &val) const
{
    m_proxyModelPrivate->setFilterRoleName(columnName);
    m_proxyModelPrivate->setFilterValue(val);

    return m_proxyModelPrivate->rowCount()>0 ? true : false;
}

// ──────── HELPER PRIVATE ──────────
QQmlPropertyMap* QModelHelper::mapperForRow(int row) const
{
    auto it = std::find_if(
        m_mappers.begin(),
        m_mappers.end(),
        [row] (const QPair<int, QQmlPropertyMap*> pair) {
            return pair.first == row;
        });

    if (it != m_mappers.end())
        return it->second;
    else
        return nullptr;
}

void QModelHelper::removeMapper(QObject* mapper)
{
    auto it = std::find_if(
        m_mappers.begin(),
        m_mappers.end(),
        [mapper] (const QPair<int, QQmlPropertyMap*> pair) {
            return pair.second == mapper;
        });

    if (it != m_mappers.end())
        m_mappers.erase(it);
}
