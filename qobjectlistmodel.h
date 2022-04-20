#ifndef QOBJECTLISTMODEL_H
#define QOBJECTLISTMODEL_H

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMetaMethod>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVector>

#include <functional>

#include "qobjectlistmodelbase.h"

#define NO_MODEL_WARNING

#ifndef NO_MODEL_WARNING
#define qolmWarning QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC,"QOLM").warning
#else
#define qolmWarning QMessageLogger().noDebug
#endif

template<class T>
class QObjectListModel : public QObjectListModelBase
{
    // ──────── CONSTRUCTOR ──────────
public:
    explicit QObjectListModel(QObject* parent = nullptr, const QList<QByteArray>& exposedRoles = {}, const QByteArray& displayRole = {}) :
        QObjectListModelBase(parent),
        m_displayRoleName(displayRole),
        m_metaObj(T::staticMetaObject)
    {
        // Keep a track of black list rolename that are not compatible with Qml, they should never be used
        static QSet<QByteArray> roleNamesBlacklist;
        if(roleNamesBlacklist.isEmpty())
        {
            roleNamesBlacklist << QByteArrayLiteral("id") << QByteArrayLiteral("index") << QByteArrayLiteral("class")
                               << QByteArrayLiteral("model") << QByteArrayLiteral("modelData");
        }

        // Set handler that handle every property changed
        static const char* HANDLER = "onItemPropertyChanged()";
        m_handler = QObjectListModelBase::metaObject()->method(QObjectListModelBase::metaObject()->indexOfMethod(HANDLER));

        // Force a display role the the role map
        if(!displayRole.isEmpty())
        {
            m_roleNames.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
        }
        // Return a pointer to the qtObject as the base Role. This point is essential
        m_roleNames.insert(baseRole(), QByteArrayLiteral("qtObject"));

        // Number of attribute declare with the Q_PROPERTY flags
        const int len = m_metaObj.propertyCount();
        // For every property in the ItemType
        for(int propertyIdx = 0, role = (baseRole() + 1); propertyIdx < len; propertyIdx++, role++)
        {
            QMetaProperty metaProp = m_metaObj.property(propertyIdx);
            const QByteArray propName = QByteArray(metaProp.name());
            // Only expose the property as a role if:
            // - It isn't blacklisted(id, index, class, model, modelData)
            // - When exposedRoles is empty we expose every property
            // - When exposedRoles isn't empty we only expose the property asked by the user
            if(!roleNamesBlacklist.contains(propName) && (exposedRoles.empty() || exposedRoles.contains(propName)))
            {
                m_roleNames.insert(role, propName);
                // If there is a notify signal associated with the Q_PROPERTY we keep a track of it for fast lookup
                if(metaProp.hasNotifySignal())
                {
                    m_signalIdxToRole.insert(metaProp.notifySignalIndex(), role);
                }
            }
            else if(roleNamesBlacklist.contains(propName))
            {
                qolmWarning() << "Can't have" << propName << "as a role name in" << qPrintable(templateClassName())
                           << ", because it's a blacklisted keywork in QML!. "
                              "Please don't use any of the following words "
                              "when declaring your Q_PROPERTY:(id, index, "
                              "class, model, modelData)";
            }
        }
    }

    QString templateClassName() const
    {
        static const QString CLASS_NAME = (QStringLiteral("QObjectModel<") + m_metaObj.className() + QStringLiteral(">"));
        return CLASS_NAME;
    }
    static const QString& emptyStr()
    {
        static const QString ret = QStringLiteral("");
        return ret;
    }
    static const QByteArray& emptyBA()
    {
        static const QByteArray ret = QByteArrayLiteral("");
        return ret;
    }
    static const QModelIndex& noParent()
    {
        static const QModelIndex ret = QModelIndex();
        return ret;
    }
    static const int& baseRole()
    {
        static const int role = Qt::UserRole;
        return role;
    }

    // ──────── ABSTRACT MODEL OVERRIDE ──────────
public:
    bool setData(const QModelIndex& modelIndex, const QVariant& value, int role) override final
    {
        bool ret = false;
        T* item = at(modelIndex.row());
        const QByteArray roleName = (role != Qt::DisplayRole ? m_roleNames.value(role, emptyBA()) : m_displayRoleName);
        if(item != nullptr && role != baseRole() && !roleName.isEmpty())
            ret = item->setProperty(roleName, value);
        return ret;
    }
    QVariant data(const QModelIndex& modelIndex, int role) const override final
    {
        QVariant ret;
        T* item = at(modelIndex.row());
        const QByteArray roleName = (role != Qt::DisplayRole ? m_roleNames.value(role, emptyBA()) : m_displayRoleName);
        if(item != nullptr && !roleName.isEmpty())
            ret.setValue(role != baseRole() ? item->property(roleName) : QVariant::fromValue(static_cast<QObject*>(item)));
        return ret;
    }

    QHash<int, QByteArray> roleNames() const override final
    {
        return m_roleNames;
    }
    int roleForName(const QByteArray& name) const override final
    {
        return m_roleNames.key(name, -1);
    }
    QByteArray roleName(int role) const override final
    {
        return m_roleNames.value(role,"");
    }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override final
    {
        return (!parent.isValid() ? m_objects.count() : 0);
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override final
    {
        return (!parent.isValid() ? roleNames().count() : 0);
    }

    // ──────── ABSTRACT MODEL PRIVATE ──────────
protected:
    void referenceItem(T* item)
    {
        if(item != nullptr)
        {
            if(!item->parent())
                item->setParent(this);

            for(QHash<int, int>::const_iterator it = m_signalIdxToRole.constBegin(); it != m_signalIdxToRole.constEnd();
                ++it)
                connect(item, item->metaObject()->method(it.key()), this, m_handler, Qt::UniqueConnection);
        }
    }
    void dereferenceItem(T* item)
    {
        if(item != nullptr)
        {
            disconnect(this, nullptr, item, nullptr);
            disconnect(item, nullptr, this, nullptr);

            if(item->parent() == this)
            {
                item->deleteLater();
            }
        }
    }
    void onItemPropertyChanged() override final
    {
        T* item = qobject_cast<T*>(sender());
        const int row = m_objects.indexOf(item);
        const int sig = senderSignalIndex();
        const int role = m_signalIdxToRole.value(sig, -1);
        if(row >= 0 && role >= 0)
        {
            const QModelIndex index = QAbstractListModel::index(row, 0, noParent());
            QVector<int> rolesList;
            rolesList.append(role);

            if(m_roleNames.value(role) == m_displayRoleName)
                rolesList.append(Qt::DisplayRole);

            Q_EMIT dataChanged(index, index, rolesList);
        }
    }
    void countInvalidate() override final
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

    // ──────── ITERATOR ──────────
public:
    using const_iterator = typename QList<T*>::const_iterator;
    const_iterator begin() const { return m_objects.begin(); }
    const_iterator end() const { return m_objects.end(); }
    const_iterator cbegin() const { return m_objects.begin(); }
    const_iterator cend() const { return m_objects.end(); }
    const_iterator constBegin() const { return m_objects.constBegin(); }
    const_iterator constEnd() const { return m_objects.constEnd(); }

    using const_reverse_iterator = typename QList<T*>::const_reverse_iterator;
    const_reverse_iterator rbegin() const { return m_objects.rbegin(); }
    const_reverse_iterator rend() const { return m_objects.rend(); }
    const_reverse_iterator crbegin() const { return m_objects.crbegin(); }
    const_reverse_iterator crend() const { return m_objects.crend(); }

    // ──────── PUBLIC C++ API ──────────
public:
    T* at(int index) const
    {
        return get(index);
    }
    T* get(int index) const
    {
        if(index < 0 || index >= m_objects.size())
        {
            qolmWarning() << templateClassName() << "The index" << index << "is out of bound.";
            return nullptr;
        }
        return m_objects.at(index);
    }

    bool contains(const T* object) const
    {
        return m_objects.contains(const_cast<T*>(object));
    }
    int indexOf(const T* object) const
    {
        if(!object)
        {
            qolmWarning() << templateClassName() << "Can't find the index of a nullptr QObject";
            return -1;
        }
        const auto index = m_objects.indexOf(const_cast<T*>(object));
        if(index < 0)
        {
            qolmWarning() << templateClassName() << "The QObject" << object << "isn't in this QObjectModel list.";
        }
        return index;
    }
    bool append(T* object)
    {
        if(object == nullptr)
        {
            qolmWarning() << templateClassName() << "Can't append a null Object";
            return false;
        }

        const int pos = m_objects.count();
        if(!objectAboutToBeInsertedNotify(object, pos))
            return false;
        beginInsertRows(noParent(), pos, pos);
        m_objects.append(object);
        referenceItem(object);
        endInsertRows();
        objectInsertedNotify(object, pos);
        return true;
    }
    bool prepend(T* object)
    {
        if(object == nullptr)
        {
            qolmWarning() << templateClassName() << "Can't prepend a null object";
            return false;
        }

        if(!objectAboutToBeInsertedNotify(object, 0))
            return false;
        beginInsertRows(noParent(), 0, 0);
        m_objects.prepend(object);
        referenceItem(object);
        endInsertRows();
        objectInsertedNotify(object, 0);
        return true;
    }
    bool insert(int index, T* object)
    {
        if(index > count())
        {
            qolmWarning() << templateClassName() << "index " << index << " is greater than count " << count() << ". "
                       << "The item will be inserted at the end of the list";
            index = count();
        }
        else if(index < 0)
        {
            qolmWarning() << templateClassName() << "index " << index << " is lower than 0. "
                       << "The item will be inserted at the beginning of the list";
            index = 0;
        }

        if(object == nullptr)
        {
            qolmWarning() << templateClassName() << "Can't insert a null Object";
            return false;
        }

        if(!objectAboutToBeInsertedNotify(object, index))
            return false;
        beginInsertRows(noParent(), index, index);
        m_objects.insert(index, object);
        referenceItem(object);
        endInsertRows();
        objectInsertedNotify(object, index);
        return true;
    }
    bool append(const QList<T*>& objectList)
    {
        if(objectList.isEmpty())
        {
            qolmWarning() << templateClassName() << "Can't append an empty list";
            return false;
        }

        const int pos = m_objects.count();
        for(int i = 0; i < objectList.count(); ++i)
        {
            if(!objectAboutToBeInsertedNotify(objectList.at(i), i + pos))
                return false;
        }

        beginInsertRows(noParent(), pos, pos + objectList.count() - 1);
        m_objects.reserve(m_objects.count() + objectList.count());
        for(const auto item: objectList)
        {
            m_objects.append(objectList);
            referenceItem(item);
        }
        endInsertRows();

        for(int i = 0; i < objectList.count(); ++i)
        {
            objectInsertedNotify(objectList.at(i), i + pos);
        }

        return true;
    }
    bool prepend(const QList<T*>& objectList)
    {
        if(objectList.isEmpty())
        {
            qolmWarning() << templateClassName() << "Can't prepend an empty list";
            return false;
        }

        for(int i = 0; i < objectList.count(); ++i)
        {
            if(!objectAboutToBeInsertedNotify(objectList.at(i), i))
                return false;
        }

        beginInsertRows(noParent(), 0, objectList.count() - 1);
        m_objects.reserve(m_objects.count() + objectList.count());
        int offset = 0;
        for(const auto item: objectList)
        {
            m_objects.insert(offset, item);
            referenceItem(item);
            offset++;
        }
        endInsertRows();

        for(int i = 0; i < objectList.count(); ++i)
        {
            objectInsertedNotify(objectList.at(i), i);
        }

        return true;
    }
    bool insert(int idx, const QList<T*>& itemList)
    {
        if(itemList.isEmpty())
        {
            qolmWarning() << templateClassName() << "Can't insert an empty list";
            return false;
        }

        for(int i = 0; i < itemList.count(); ++i)
        {
            if(!objectAboutToBeInsertedNotify(itemList.at(i), i + idx))
                return false;
        }

        beginInsertRows(noParent(), idx, idx + itemList.count() - 1);
        m_objects.reserve(m_objects.count() + itemList.count());
        int offset = 0;
        for(const auto item: itemList)
        {
            m_objects.insert(idx + offset, item);
            referenceItem(item);
            offset++;
        }
        endInsertRows();

        for(int i = 0; i < itemList.count(); ++i)
        {
            objectInsertedNotify(itemList.at(i), i + idx);
        }

        return true;
    }
    bool move(int from, int to) override final
    {
        if(from < 0 || from >= count())
        {
            qolmWarning() << templateClassName() << "'From'" << from << "is out of bound";
            return false;
        }

        const auto clampedTo = std::clamp(to, 0, count() - 1);
        if(clampedTo != to)
        {
            qolmWarning() << templateClassName() << "'to'" << to << " in move operation have been clamped to" << clampedTo;
            to = clampedTo;
            if(from == to)
            {
                qolmWarning() << templateClassName() << "Can't move object from" << from << "to" << to << "because from == to";
                return false;
            }
        }

        const auto object = m_objects.at(from);
        if(!objectAboutToBeMovedNotify(object, from, to))
            return false;
        beginMoveRows(noParent(), from, from, noParent(), (from < to ? to + 1 : to));
        m_objects.move(from, to);
        endMoveRows();
        objectMovedNotify(object, from, to);

        return true;
    }
    bool remove(const T* object)
    {
        if(object == nullptr)
        {
            qolmWarning() << templateClassName() << "Fail to remove nullptr object from QObjectModel<" << m_metaObj.className() << ">";
            return false;
        }

        return remove(indexOf(object));
    }
    bool remove(const QList<T*>& objects)
    {
        bool ret=true;
        for(const auto* object: objects)
        {
            if(!remove(object))
                ret = false;
        }
        return ret;
    }
    bool remove(int index, int count = 1)
    {
        if(index < 0 || (index + count - 1) >= m_objects.size())
        {
            qolmWarning() << templateClassName() << "Can't remove an object whose index is out of bound";
            return false;
        }

        QList<T*> tempList;
        for(const auto item: *this)
        {
            tempList.append(item);
        }
        for(int i = 0; i < count; ++i)
        {
            if(!objectAboutToBeRemovedNotify(tempList.at(index + i), index + i))
                return false;
        }

        beginRemoveRows(noParent(), index, index + count - 1);
        for(int i = 0; i < count; ++i)
        {
            T* item = m_objects.takeAt(index);
            dereferenceItem(item);
        }
        endRemoveRows();
        for(int i = 0; i < count; ++i)
        {
            objectRemovedNotify(tempList.at(index + i), index + i);
        }

        return true;
    }
    bool clear() override final
    {
        if(m_objects.isEmpty())
            return true;

        QList<T*> tempList;
        for(int i = 0; i < m_objects.count(); ++i)
        {
            if(!objectAboutToBeRemovedNotify(m_objects.at(i), i))
                return false;
        }
        beginRemoveRows(noParent(), 0, m_objects.count() - 1);
        for(const auto item: *this)
        {
            dereferenceItem(item);
            tempList.append(item);
        }
        m_objects.clear();
        endRemoveRows();

        for(int i = 0; i < tempList.count(); ++i)
        {
            objectRemovedNotify(tempList.at(i), i);
        }

        return true;
    }
    T* first() const
    {
        if(m_objects.isEmpty())
        {
            qolmWarning() << templateClassName() << "The first element of an empty list doesn't exist !";
            return nullptr;
        }

        return m_objects.first();
    }
    T* last() const
    {
        if(m_objects.isEmpty())
        {
            qolmWarning() << templateClassName() << "The last element of an empty list doesn't exist !";
            return nullptr;
        }

        return m_objects.last();
    }

    const QList<T*>& toList() const
    {
        return m_objects;
    }

    // ──────── QML OVERRIDE API ──────────
public:
    QObject* at(QJSValue index) const override
    {
        const auto i = index.toInt();
        return get(i);
    }
    QObject* get(QJSValue index) const override
    {
        const auto i = index.toInt();
        return get(i);
    }
    bool append(QJSValue value) override final
    {
        if(value.isQObject())
        {
            const auto castObject = qobject_cast<T*>(value.toQObject());
            if(castObject)
            {
                return append(castObject);
            }
            else
            {
                qolmWarning() << templateClassName() << ": Fail to append" << value.toString() << ", item isn't a"
                           << m_metaObj.className() << "derived class";
                return false;
            }
        }
        else if(value.isArray())
        {
            QList<T*> listToAppend;
            for(int i = 0; i < value.property("length").toInt(); ++i)
            {
                const auto object = value.property(i);
                if(object.isQObject())
                {
                    const auto castObject = qobject_cast<T*>(object.toQObject());

                    if(castObject)
                    {
                        listToAppend.append(castObject);
                    }
                    else
                    {
                        qolmWarning() << templateClassName() << ": Fail to append " << object.toString()
                                   << ", item isn't a" << m_metaObj.className() << "derived class";
                        return false;
                    }
                }
                else
                {
                    qolmWarning() << templateClassName() << ": Fail to append " << object.toString()
                               << ", item isn't QObject";
                    return false;
                }
            }
            if(!listToAppend.isEmpty())
                return append(listToAppend);
        }

        qolmWarning() << templateClassName() << ": Fail to append" << value.toString()
                   << ", item isn't a QObject, an array of QObject";
        return false;
    }
    bool prepend(QJSValue value) override final
    {
        if(value.isQObject())
        {
            const auto castObject = qobject_cast<T*>(value.toQObject());
            if(castObject)
            {
                return prepend(castObject);
            }
            else
            {
                qolmWarning() << templateClassName() << ": Fail to prepend" << value.toString() << ", item isn't a"
                           << m_metaObj.className() << "derived class";
                return false;
            }
        }
        else if(value.isArray())
        {
            QList<T*> listToPrepend;
            for(int i = 0; i < value.property("length").toInt(); ++i)
            {
                const auto object = value.property(i);
                if(object.isQObject())
                {
                    const auto castObject = qobject_cast<T*>(object.toQObject());

                    if(castObject)
                    {
                        listToPrepend.append(castObject);
                    }
                    else
                    {
                        qolmWarning() << templateClassName() << ": Fail to prepend " << object.toString()
                                   << ", item isn't a" << m_metaObj.className() << "derived class";
                        return false;
                    }
                }
                else
                {
                    qolmWarning() << templateClassName() << ": Fail to prepend " << object.toString()
                               << ", item isn't QObject";
                    return false;
                }
            }
            if(!listToPrepend.isEmpty())
                return prepend(listToPrepend);
        }

        qolmWarning() << templateClassName() << ": Fail to prepend" << value.toString()
                   << ", item isn't a QObject, an array of QObject";
        return false;
    }
    bool insert(int idx, QJSValue value) override final
    {
        if(value.isQObject())
        {
            const auto castObject = qobject_cast<T*>(value.toQObject());
            if(castObject)
            {
                return insert(idx, castObject);
            }
            else
            {
                qolmWarning() << templateClassName() << ": Fail to insert" << value.toString() << ", item isn't a"
                           << m_metaObj.className() << "derived class";
                return false;
            }
        }
        else if(value.isArray())
        {
            QList<T*> listToInsert;
            for(int i = 0; i < value.property("length").toInt(); ++i)
            {
                const auto object = value.property(i);
                if(object.isQObject())
                {
                    const auto castObject = qobject_cast<T*>(object.toQObject());

                    if(castObject)
                    {
                        listToInsert.append(castObject);
                    }
                    else
                    {
                        qolmWarning() << templateClassName() << ": Fail to insert " << object.toString()
                                   << ", item isn't a" << m_metaObj.className() << "derived class";
                        return false;
                    }
                }
                else
                {
                    qolmWarning() << templateClassName() << ": Fail to insert " << object.toString()
                               << ", item isn't QObject";
                    return false;
                }
            }
            if(!listToInsert.isEmpty())
                return insert(idx, listToInsert);
        }

        qolmWarning() << templateClassName() << ": Fail to insert" << value.toString()
                   << ", item isn't a QObject, an array of QObject";
        return false;
    }
    bool remove(QJSValue value) override final
    {
        if(value.isNumber())
        {
            const auto index = int(value.toNumber());
            return remove(index);
        }
        else if(value.isQObject())
        {
            const auto castObject = qobject_cast<T*>(value.toQObject());
            if(castObject)
            {
                return remove(castObject);
            }
            else
            {
                qolmWarning() << templateClassName() << ": Fail to remove" << value.toString() << ", item isn't a"
                           << m_metaObj.className() << "derived class";
                return false;
            }
        }
        else if(value.isArray())
        {
            QList<T*> listToRemove;
            for(int i = 0; i < value.property("length").toInt(); ++i)
            {
                const auto object = value.property(i);
                if(object.isQObject())
                {
                    const auto castObject = qobject_cast<T*>(object.toQObject());

                    if(castObject)
                    {
                        listToRemove.append(castObject);
                    }
                    else
                    {
                        qolmWarning() << templateClassName() << ": Fail to insert " << object.toString()
                                   << ", item isn't a" << m_metaObj.className() << "derived class";
                        return false;
                    }
                }
                else
                {
                    qolmWarning() << templateClassName() << ": Fail to insert " << object.toString()
                               << ", item isn't QObject";
                    return false;
                }
            }
            if(!listToRemove.isEmpty())
                return remove(listToRemove);
        }

        qolmWarning() << templateClassName() << ": Fail to remove" << value.toString()
                   << ", item isn't a QObject, an array of QObject or a number";
        return false;
    }
    bool contains(QJSValue value) const override final
    {
        if(value.isQObject())
        {
            const auto castObject = qobject_cast<T*>(value.toQObject());
            if(castObject)
            {
                return contains(castObject);
            }
            else
            {
                qolmWarning() << templateClassName() << ": Fail to get indexOf" << value.toString() << ", item isn't a"
                           << m_metaObj.className() << "derived class";
                return false;
            }
        }

        qolmWarning() << templateClassName() << ": Fail to get indexOf" << value.toString()
                   << ", item isn't a QObject";
        return false;
    }
    int indexOf(QJSValue value) const override final
    {
        if(value.isQObject())
        {
            const auto castObject = qobject_cast<T*>(value.toQObject());
            if(castObject)
            {
                return indexOf(castObject);
            }
            else
            {
                qolmWarning() << templateClassName() << ": Fail to get indexOf" << value.toString() << ", item isn't a"
                           << m_metaObj.className() << "derived class";
                return -1;
            }
        }

        qolmWarning() << templateClassName() << ": Fail to get indexOf" << value.toString()
                   << ", item isn't a QObject";
        return -1;
    }
    bool moveUp(const int index) override final
    {
        // Move index to index-1
        if(index <= 0 || index >= count())
        {
            qolmWarning() << templateClassName() << "The index is the first of the list or index is out of bound";
            return false;
        }

        return move(index, index - 1);
    }
    bool moveDown(const int index) override final
    {
        // Move index to index+1
        if(!(count() &&  // There is a least one entry
            index >= 0 &&  // We can be from the first
            index < (count() - 1))  // To the last one minus 1
        )
        {
            qolmWarning() << templateClassName() << "The index is the last of the list or index is out of bound";
            return false;
        }

        return move(index, index + 1);
    }

    // ──────── DEFAULT CONTENT ──────────
protected:
    void content_append(QObject* child) override final
    {
        const auto object = qobject_cast<T*>(child);
        if(object)
        {
            m_defaultObjects.append(object);
            append(object);
        }
        else
        {
            qolmWarning() << templateClassName() << ": Fail to append default child " << child->objectName()
                       << ", that isn't a" << m_metaObj.className() << "derived class";
        }
    }
    int content_count() override final
    {
        return m_defaultObjects.count();
    }
    QObject* content_at(int index) override final
    {
        if(m_defaultObjects.count() < index)
            return nullptr;

        return m_defaultObjects.at(index);
    }
    void content_clear() override final
    {
        remove(m_defaultObjects);
        m_defaultObjects.clear();
    }
    void content_replace(int index, QObject* child) override final
    {
        const auto object = qobject_cast<T*>(child);
        if(!object)
        {
            qolmWarning() << templateClassName() << ": Fail to append default child " << child->objectName()
                       << ", that isn't a" << m_metaObj.className() << "derived class";
        }

        if(m_defaultObjects.count() < index)
            return;

        const auto previousObject = m_defaultObjects.at(index);
        const auto previousObjectIndex = indexOf(previousObject);
        if(previousObject)
            remove(previousObject);

        m_defaultObjects.replace(index, object);

        if(child)
            insert(previousObjectIndex, object);
    }
    void content_removeLast() override final
    {
        if(m_defaultObjects.isEmpty())
            return;

        const auto objectToRemove = m_defaultObjects.last();
        remove(objectToRemove);
        m_defaultObjects.removeLast();
    }

    // ──────── PRIVATE OBSERVER API ──────────
private:
    bool objectAboutToBeInsertedNotify(T* object, int index)
    {
        return onObjectAboutToBeInserted(object, index);
    }
    void objectInsertedNotify(T* object, int index)
    {
        onObjectInserted(object, index);
        Q_EMIT objectInserted(object, index);
    }
    bool objectAboutToBeMovedNotify(T* object, int src, int dest)
    {
        return onObjectAboutToBeMoved(object, src, dest);
    }
    void objectMovedNotify(T* object, int src, int dest)
    {
        onObjectMoved(object, src, dest);
        Q_EMIT objectMoved(object, src, dest);
    }
    bool objectAboutToBeRemovedNotify(T* object, int index)
    {
        return onObjectAboutToBeRemoved(object, index);
    }
    void objectRemovedNotify(T* object, int index)
    {
        onObjectRemoved(object, index);
        Q_EMIT objectRemoved(object, index);
    }

    // ──────── OVERRIDE OBSERVER API ──────────
protected:
    virtual bool onObjectAboutToBeInserted(T* object, int index)
    {
        Q_UNUSED(object)
        Q_UNUSED(index)
        return true;
    }
    virtual void onObjectInserted(T* object, int index)
    {
        Q_UNUSED(object)
        Q_UNUSED(index)
    }
    virtual bool onObjectAboutToBeMoved(T* object, int src, int dest)
    {
        Q_UNUSED(object)
        Q_UNUSED(src)
        Q_UNUSED(dest)
        return true;
    }
    virtual void onObjectMoved(T* object, int src, int dest)
    {
        Q_UNUSED(object)
        Q_UNUSED(src)
        Q_UNUSED(dest)
    }
    virtual bool onObjectAboutToBeRemoved(T* object, int index)
    {
        Q_UNUSED(object)
        Q_UNUSED(index)
        return true;
    }
    virtual void onObjectRemoved(T* object, int index)
    {
        Q_UNUSED(object)
        Q_UNUSED(index)
    }

    // ──────── PUBLIC OBSERVER API ──────────
public:
    QMetaObject::Connection onInserted(QObject* receiver, std::function<void(T* object, int index)> callback)
    {
        if(!receiver)
        {
            qolmWarning() << "QObjectModel::onInserted: Fail to connect to nullptr receiver";
            return {};
        }

        if(!callback)
        {
            qolmWarning() << "QObjectModel::onInserted: Fail to connect empty lambda";
            return {};
        }

        return connect(this, &QObjectListModelBase::objectInserted, receiver,
            [callback](QObject* qobject, int index)
            {
                T* object = qobject_cast<T*>(qobject);
                Q_ASSERT(object);
                callback(object, index);
            });
    }
    QMetaObject::Connection onInserted(QObject* receiver, std::function<void(T* object)> callback)
    {
        if(!callback)
        {
            qolmWarning() << "QObjectModel::onInserted: Fail to connect empty lambda";
            return {};
        }

        return onInserted(receiver, [callback](T* object, int) { callback(object); });
    }
    QMetaObject::Connection onInserted(std::function<void(T* object, int index)> callback)
    {
        return onInserted(this, callback);
    }
    QMetaObject::Connection onInserted(std::function<void(T* object)> callback)
    {
        return onInserted(this, callback);
    }

    QMetaObject::Connection onRemoved(QObject* receiver, std::function<void(T* object, int index)> callback)
    {
        if(!receiver)
        {
            qolmWarning() << "QObjectModel::onRemoved: Fail to connect to nullptr receiver";
            return {};
        }

        if(!callback)
        {
            qolmWarning() << "QObjectModel::onRemoved: Fail to connect empty lambda";
            return {};
        }

        return connect(this, &QObjectListModelBase::objectRemoved, receiver,
            [callback](QObject* qobject, int index)
            {
                T* object = qobject_cast<T*>(qobject);
                Q_ASSERT(object);
                callback(object, index);
            });
    }
    QMetaObject::Connection onRemoved(QObject* receiver, std::function<void(T* object)> callback)
    {
        if(!callback)
        {
            qolmWarning() << "QObjectModel::onRemoved: Fail to connect empty lambda";
            return {};
        }

        return onRemoved(receiver, [callback](T* object, int) { callback(object); });
    }
    QMetaObject::Connection onRemoved(std::function<void(T* object, int index)> callback)
    {
        return onRemoved(this, callback);
    }
    QMetaObject::Connection onRemoved(std::function<void(T* object)> callback)
    {
        return onRemoved(this, callback);
    }

    QMetaObject::Connection onMoved(QObject* receiver, std::function<void(T* object, int from, int to)> callback)
    {
        if(!receiver)
        {
            qolmWarning() << "QObjectModel::onMoved: Fail to connect to nullptr receiver";
            return {};
        }

        if(!callback)
        {
            qolmWarning() << "QObjectModel::onMoved: Fail to connect empty lambda";
            return {};
        }

        return connect(this, &QObjectListModelBase::objectMoved, receiver,
            [callback](QObject* qobject, int from, int to)
            {
                T* object = qobject_cast<T*>(qobject);
                Q_ASSERT(object);
                callback(object, from, to);
            });
    }
    QMetaObject::Connection onMoved(std::function<void(T* object, int from, int to)> callback)
    {
        return onMoved(this, callback);
    }

    // ──────── ATTRIBUTES ──────────
private:
    int m_count=0;
    QByteArray m_displayRoleName;
    QMetaObject m_metaObj;
    QMetaMethod m_handler;
    QHash<int, QByteArray> m_roleNames;
    QHash<int, int> m_signalIdxToRole;
    QList<T*> m_objects;
    QList<T*> m_defaultObjects;
};

#endif
