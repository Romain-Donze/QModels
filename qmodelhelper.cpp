#include "qmodelhelper.h"
#include "qmodels_log.h"

#include <QSortFilterProxyModel>
#include <QElapsedTimer>
#include <QDateTime>

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
        connect(model, &QAbstractItemModel::columnsRemoved, this, &QModelHelperPropertyMap::onColumnsRemoved);
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
// QModelHelper
//────────────────────────────────────────────────────────────────────────────────────────────────

// ──────── CONSTRUCTOR ──────────
QModelHelper::QModelHelper(QAbstractItemModel* object) :
    QAbstractItemModel(object),
    m_sourceModel(object),
    m_backupModel(new QStandardItemModel(this))
{
    if (!object || !m_sourceModel)
    {
        QMODELSLOG_CRITICAL()<<object<<m_sourceModel;
        QMODELSLOG_FATAL("QModelHelper must be attached to a QAbstractItemModel*");
    }
    else
    {
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
        QMODELSLOG_CRITICAL()<<object<<model;
        QMODELSLOG_FATAL("QModelHelper must be attached to a QAbstractItemModel*");
        return nullptr;
    }

    QModelHelper* helper = model->findChild<QModelHelper*>(QString(), Qt::FindDirectChildrenOnly);
    if(!helper)
    {
        helper = new QModelHelper(model);
        QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
    }

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

// ──────── PUBLIC API ──────────
QQmlPropertyMap* QModelHelper::map(int row, int column, const QModelIndex& parent)
{
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

// ──────── PUBLIC API ──────────
bool QModelHelper::set(int row, const QVariantMap& pArray)
{
    return set(m_sourceModel, row, pArray);
}

bool QModelHelper::setProperty(int row, const QString& property, const QVariant& value)
{
    return setProperty(m_sourceModel, row, property, value);
}

bool QModelHelper::setProperties(const QString& property, const QVariant& value)
{
    return setProperties(m_sourceModel, property, value);
}

bool QModelHelper::setProperties(const QList<int>& indexes, const QString& property, const QVariant& value)
{
    return setProperties(m_sourceModel, indexes, property, value);
}

QVariantMap QModelHelper::get(int row, const QStringList roles) const
{
    return get(m_sourceModel, row, roles);
}

QVariant QModelHelper::getProperty(int row, const QString& property) const
{
    return getProperty(m_sourceModel, row, property);
}

QVariantList QModelHelper::getProperties(const QString& property) const
{
    return getProperties(m_sourceModel, property);
}

QVariantList QModelHelper::getProperties(const QList<int>& indexes, const QString& property) const
{
    return getProperties(m_sourceModel, indexes, property);
}

int QModelHelper::indexOf(const QString &columnName, const QVariant &val, bool isSorted) const
{
    return indexOf(m_sourceModel, columnName, val, isSorted);
}

QList<int> QModelHelper::indexesOf(const QString &columnName, const QVariant &val) const
{
    return indexesOf(m_sourceModel, columnName, val);
}

int QModelHelper::count(const QString &columnName, const QVariant &val) const
{
    return count(m_sourceModel, columnName, val);
}

bool QModelHelper::contains(const QString &columnName, const QVariant &val, bool isSorted) const
{
    return contains(m_sourceModel, columnName, val, isSorted);
}

bool QModelHelper::isSorted(const QString &columnName) const
{
    return isSorted(m_sourceModel, columnName);
}

bool QModelHelper::equals(QAbstractItemModel* model) const
{
    return equals(m_sourceModel, model);
}

QVariantList QModelHelper::toVariantList() const
{
    return toVariantList(m_sourceModel);
}

const QVariantList& QModelHelper::backup() const
{
    m_backup=toVariantList(m_sourceModel);
    return m_backup;
}

bool QModelHelper::clearBackup()
{
    m_backup.clear();
    return true;
}

bool QModelHelper::hasChanged() const
{
    if(count() != m_backup.count())
        return true;

    for(int i=0; i<count(); ++i)
    {
        const QVariantMap& a = get(i);
        const QVariantMap& b = m_backup.at(i).toMap();

        const QList<QString> aProps=a.keys();
        const QList<QString> bProps=b.keys();

        if (aProps.count() != bProps.count())
            return true;

        for(const QString& prop: aProps)
        {
            if (a.value(prop) != b.value(prop))
                return true;
        }
    }

    return false;
}

// ──────── PUBLIC STATIC API ──────────
int QModelHelper::roleForName(QAbstractItemModel* model, const QByteArray& name)
{
    if(!model)
        return -1;

    return model->roleNames().key(name, -1);
}

QByteArray QModelHelper::roleName(QAbstractItemModel* model, int role)
{
    if(!model)
        return QByteArray();

    return model->roleNames().value(role,"");
}

bool QModelHelper::set(QAbstractItemModel* model, int row, const QVariantMap& pArray)
{
    if(!model)
        return false;

    bool aRet=false;
    if (row >= count(model) || row < 0)
        return aRet;
    for (QVariantMap::const_iterator it = pArray.begin(); it != pArray.end(); ++it)
    {
        aRet = setProperty(model, row,it.key(),it.value());
    }

    return aRet;
}

bool QModelHelper::setProperty(QAbstractItemModel* model, int row, const QString& property, const QVariant& value)
{
    if(!model)
        return false;

    int roleIndex = roleForName(model, property.toUtf8());

    if(roleIndex<0)
        return false;

    return model->setData(model->index(row, 0), value, roleIndex);
}

bool QModelHelper::setProperties(QAbstractItemModel* model, const QString& property, const QVariant& value)
{
    if(!model)
        return false;

    bool aRet = false;

    int roleIndex = roleForName(model, property.toUtf8());
    for(int i=count(model)-1;i>=0;i--)
    {
        aRet = model->setData(model->index(i, 0), value, roleIndex);
    }

    return aRet;
}

bool QModelHelper::setProperties(QAbstractItemModel* model, const QList<int>& indexes, const QString& property, const QVariant& value)
{
    if(!model)
        return false;

    bool aRet = false;

    int roleIndex = roleForName(model, property.toUtf8());
    for(int index: indexes)
    {
        aRet = model->setData(model->index(index, 0), value, roleIndex);
    }

    return aRet;
}

QVariantMap QModelHelper::get(QAbstractItemModel* model, int row, const QStringList roles)
{
    if(!model)
        return QVariantMap();

    QHash<int,QByteArray> names = model->roleNames();
    if(!roles.isEmpty())
    {
        QHash<int,QByteArray> tmpNames=names;
        names.clear();
        for(const QString& role: roles)
        {
            const QByteArray roleName = role.toUtf8();
            names[tmpNames.key(roleName)]=roleName;
        }
    }

    QVariantMap map;
    QModelIndex modelIndex = model->index(row, 0);
    for (QHash<int, QByteArray>::iterator it = names.begin(); it != names.end(); ++it)
    {
        map.insert(it.value(), model->data(modelIndex, it.key()));
    }

    return map;
}

QVariant QModelHelper::getProperty(QAbstractItemModel* model, int row, const QString& property)
{
    if(!model)
        return QVariant();

    int roleIndex = roleForName(model, property.toUtf8());

    if(roleIndex<0)
        return QVariant();

    return model->data(model->index(row, 0), roleIndex);
}

QVariantList QModelHelper::getProperties(QAbstractItemModel* model, const QString& property)
{
    if(!model)
        return QVariantList();

    QVariantList ret;
    int roleIndex = roleForName(model, property.toUtf8());

    if(roleIndex<0)
        return ret;

    ret.reserve(model->rowCount());
    for(int i=0;i<count(model);++i)
    {
        ret+=model->data(model->index(i, 0), roleIndex);
    }

    return ret;
}

QVariantList QModelHelper::getProperties(QAbstractItemModel* model, const QList<int>& indexes, const QString& property)
{
    if(!model)
        return QVariantList();

    int roleIndex = roleForName(model, property.toUtf8());

    QVariantList ret;
    if(roleIndex<0)
        return ret;

    ret.reserve(indexes.size());
    for(int index: indexes)
    {
        ret+=model->data(model->index(index, 0), roleIndex);
    }

    return ret;
}

int QModelHelper::indexOf(QAbstractItemModel* model, int role, const QVariant& val, bool isSorted)
{
    int ret = -1;
    if(!model)
        return ret;

    if(isSorted)
    {
        int lower = 0;
        int upper = (int (model->rowCount()) -1);

        while (lower <= upper)
        {
            const int middle = (lower + (upper - lower) / 2);
            const QVariant& var = model->data(model->index(middle,0), role);
            if (var == val)
            {
                ret = middle;
                break;
            }
            else if (var < val)
            {
                lower = middle + 1;
            }
            else
            {
                upper = middle - 1;
            }
        }
    }
    else
    {
        for(int row=0; row<model->rowCount(); ++row)
        {
            const QVariant& var = model->data(model->index(row,0), role);
            if(var == val)
            {
                ret = row;
                break;
            }
        }
    }

    return ret;
}
QList<int> QModelHelper::indexesOf(QAbstractItemModel* model, int role, const QVariant& val)
{
    if(!model)
        return QList<int>();

    QList<int> ret;

    for(int i=0; i<model->rowCount(); ++i)
    {
        const QVariant& var = model->data(model->index(i,0), role);
        if(var == val)
        {
            ret += i;
        }
    }

    return ret;
}
int QModelHelper::count(QAbstractItemModel* model, int role, const QVariant& val)
{
    return indexesOf(model, role, val).size();
}
bool QModelHelper::contains(QAbstractItemModel* model, int role, const QVariant& val, bool isSorted)
{
    return indexOf(model, role, val, isSorted) >= 0;
}
bool QModelHelper::isSorted(QAbstractItemModel* model, int role)
{
    if(!model)
        return false;

    for(int i=0; i<model->rowCount(); ++i)
    {
        if(i>0)
        {
            const QVariant& var = model->data(model->index(i,0), role);
            const QVariant& varPrev = model->data(model->index(i-1,0), role);
            if(var < varPrev)
            {
                return false;
            }
        }
    }

    return true;
}

int QModelHelper::indexOf(QAbstractItemModel* model, const QString &columnName, const QVariant& val, bool isSorted)
{
    int roleIndex = roleForName(model, columnName.toUtf8());
    return indexOf(model, roleIndex, val, isSorted);
}
QList<int> QModelHelper::indexesOf(QAbstractItemModel* model, const QString &columnName, const QVariant& val)
{
    int roleIndex = roleForName(model, columnName.toUtf8());
    return indexesOf(model, roleIndex, val);
}
int QModelHelper::count(QAbstractItemModel* model, const QString &columnName, const QVariant& val)
{
    int roleIndex = roleForName(model, columnName.toUtf8());
    return count(model, roleIndex, val);
}
bool QModelHelper::contains(QAbstractItemModel* model, const QString& columnName, const QVariant& val, bool isSorted)
{
    int roleIndex = roleForName(model, columnName.toUtf8());
    return contains(model, roleIndex, val, isSorted);
}
bool QModelHelper::isSorted(QAbstractItemModel* model, const QString& columnName)
{
    int roleIndex = roleForName(model, columnName.toUtf8());
    return isSorted(model, roleIndex);
}

bool QModelHelper::equals(QAbstractItemModel* srcModel, QAbstractItemModel* compModel)
{
    if(srcModel==nullptr || compModel==nullptr)
        return false;

    if(srcModel->rowCount() != compModel->rowCount())
        return false;

    if(srcModel->roleNames().count() != compModel->roleNames().count())
        return false;

    for(int i=0; i<srcModel->rowCount(); ++i)
    {
        const QVariantMap& a = QModelHelper::get(srcModel, i);
        const QVariantMap& b = QModelHelper::get(compModel, i);

        const QList<QString> aProps=a.keys();
        const QList<QString> bProps=b.keys();

        if (aProps.count() != bProps.count())
            return false;

        for(const QString& prop: aProps)
        {
            if (a.value(prop) != b.value(prop))
                return false;
        }
    }

    return true;
}

QVariantList QModelHelper::toVariantList(QAbstractItemModel* model)
{
    QVariantList variantList;
    variantList.reserve(model->rowCount());

    const QHash<int,QByteArray> names = model->roleNames();
    for(int i=0; i<model->rowCount(); ++i)
    {
        QVariantMap map;
        const QModelIndex modelIndex = model->index(i, 0);
        for (QHash<int, QByteArray>::const_iterator it = names.begin(); it != names.end(); ++it)
        {
            map.insert(it.value(), model->data(modelIndex, it.key()));
        }
        variantList.append(map);
    }

    return variantList;
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
