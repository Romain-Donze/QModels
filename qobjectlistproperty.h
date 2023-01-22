#ifndef QOBJECTLISTPROPERTY_H
#define QOBJECTLISTPROPERTY_H

#include <QDebug>
#include <QQmlListProperty>
#include <QQmlEngine>

#include "qmodels_log.h"

#define Q_CONSTANT_OLP_PROPERTY(TYPE, name, Name, ...) \
    private:    Q_PROPERTY (QQmlListProperty<TYPE> name READ get##Name CONSTANT FINAL) \
    public:     const QObjectListProperty<TYPE>& get##Name (void) const { return m_##name; } \
    protected:  QObjectListProperty<TYPE> m_##name; \
    private:

template<class T>
class QObjectListProperty : public QQmlListProperty<T>
{
    // ──────── CONSTRUCTOR ──────────
public:
    using QmlListProperty = QQmlListProperty<T>;

    explicit QObjectListProperty(QObject* object) :
        QmlListProperty(
              object,
              this,
              &QObjectListProperty<T>::list_append,
              &QObjectListProperty<T>::list_count,
              &QObjectListProperty<T>::list_at,
              &QObjectListProperty<T>::list_clear,
              &QObjectListProperty<T>::list_replace,
              &QObjectListProperty<T>::list_removeLast)
    {

    }

    QString templateClassName() const
    {
        static const QString CLASS_NAME = (QStringLiteral("QObjectListProperty<") + T::staticMetaObject.className() + QStringLiteral(">"));
        return CLASS_NAME;
    }

    // ──────── ITERATOR ──────────
public:
    using const_iterator = typename QList<T*>::const_iterator;
    const_iterator begin() const { return m_content.begin(); }
    const_iterator end() const { return m_content.end(); }
    const_iterator cbegin() const { return m_content.begin(); }
    const_iterator cend() const { return m_content.end(); }
    const_iterator constBegin() const { return m_content.constBegin(); }
    const_iterator constEnd() const { return m_content.constEnd(); }

    using const_reverse_iterator = typename QList<T*>::const_reverse_iterator;
    const_reverse_iterator rbegin() const { return m_content.rbegin(); }
    const_reverse_iterator rend() const { return m_content.rend(); }
    const_reverse_iterator crbegin() const { return m_content.crbegin(); }
    const_reverse_iterator crend() const { return m_content.crend(); }

    inline T * operator[] (const int idx) const {
        return (idx >= 0 && idx < m_content.length() ? m_content.at(idx) : nullptr);
    }

    // ──────── PUBLIC C++ API ──────────
public:
    int count(void) const { return m_content.count(); }
    int length(void) const { return m_content.length(); }
    int size(void) const { return m_content.size(); }
    bool isEmpty(void) const { return m_content.isEmpty(); }

    T* at(int index) const
    {
        return get(index);
    }
    T* get(int index) const
    {
        if(index < 0 || index >= m_content.size())
        {
            QMODELSLOG_WARNING() << templateClassName() << "The index" << index << "is out of bound.";
            return nullptr;
        }
        return m_content.at(index);
    }

    bool contains(const T* object) const
    {
        return m_content.contains(const_cast<T*>(object));
    }
    int indexOf(const T* object) const
    {
        if(!object)
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't find the index of a nullptr QObject";
            return -1;
        }
        const auto index = m_content.indexOf(const_cast<T*>(object));
        if(index < 0)
        {
            QMODELSLOG_WARNING() << templateClassName() << "The QObject" << object << "isn't in this list.";
        }
        return index;
    }
    bool append(T* object)
    {
        if(object == nullptr)
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't append a null Object";
            return false;
        }

        m_content.append(object);
        referenceItem(object);
        return true;
    }
    bool prepend(T* object)
    {
        if(object == nullptr)
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't prepend a null object";
            return false;
        }

        m_content.prepend(object);
        referenceItem(object);
        return true;
    }
    bool insert(int index, T* object)
    {
        if(index > count())
        {
            QMODELSLOG_WARNING() << templateClassName() << "index " << index << " is greater than count " << count() << ". "
                       << "The item will be inserted at the end of the list";
            index = count();
        }
        else if(index < 0)
        {
            QMODELSLOG_WARNING() << templateClassName() << "index " << index << " is lower than 0. "
                       << "The item will be inserted at the beginning of the list";
            index = 0;
        }

        if(object == nullptr)
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't insert a null Object";
            return false;
        }

        m_content.insert(index, object);
        referenceItem(object);
        return true;
    }
    bool replace(int index, T* object)
    {
        if(index > count())
        {
            QMODELSLOG_WARNING() << templateClassName() << "index " << index << " is greater than count " << count() << ". "
                       << "The item will be inserted at the end of the list";
            index = count();
        }
        else if(index < 0)
        {
            QMODELSLOG_WARNING() << templateClassName() << "index " << index << " is lower than 0. "
                       << "The item will be inserted at the beginning of the list";
            index = 0;
        }

        if(object == nullptr)
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't insert a null Object";
            return false;
        }

        remove(index);
        insert(index, object);
        return true;
    }
    bool append(const QList<T*>& objectList)
    {
        if(objectList.isEmpty())
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't append an empty list";
            return false;
        }

        m_content.reserve(m_content.count() + objectList.count());
        for(const auto item: objectList)
        {
            m_content.append(objectList);
            referenceItem(item);
        }

        return true;
    }
    bool prepend(const QList<T*>& objectList)
    {
        if(objectList.isEmpty())
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't prepend an empty list";
            return false;
        }

        for(int i = 0; i < objectList.count(); ++i)
        {
            if(!objectAboutToBeInsertedNotify(objectList.at(i), i))
                return false;
        }

        m_content.reserve(m_content.count() + objectList.count());
        int offset = 0;
        for(const auto item: objectList)
        {
            m_content.insert(offset, item);
            referenceItem(item);
            offset++;
        }

        return true;
    }
    bool insert(int idx, const QList<T*>& itemList)
    {
        if(itemList.isEmpty())
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't insert an empty list";
            return false;
        }

        for(int i = 0; i < itemList.count(); ++i)
        {
            if(!objectAboutToBeInsertedNotify(itemList.at(i), i + idx))
                return false;
        }

        m_content.reserve(m_content.count() + itemList.count());
        int offset = 0;
        for(const auto item: itemList)
        {
            m_content.insert(idx + offset, item);
            referenceItem(item);
            offset++;
        }

        return true;
    }
    bool move(int from, int to)
    {
        if(from < 0 || from >= count())
        {
            QMODELSLOG_WARNING() << templateClassName() << "'From'" << from << "is out of bound";
            return false;
        }

        const auto clampedTo = std::clamp(to, 0, count() - 1);
        if(clampedTo != to)
        {
            QMODELSLOG_WARNING() << templateClassName() << "'to'" << to << " in move operation have been clamped to" << clampedTo;
            to = clampedTo;
            if(from == to)
            {
                QMODELSLOG_WARNING() << templateClassName() << "Can't move object from" << from << "to" << to << "because from == to";
                return false;
            }
        }

        m_content.move(from, to);

        return true;
    }
    bool remove(const T* object)
    {
        if(object == nullptr)
        {
            QMODELSLOG_WARNING() << templateClassName() << "Fail to remove nullptr object from" << templateClassName();
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
        if(index < 0 || (index + count - 1) >= m_content.size())
        {
            QMODELSLOG_WARNING() << templateClassName() << "Can't remove an object whose index is out of bound";
            return false;
        }

        for(int i = 0; i < count; ++i)
        {
            T* item = m_content.takeAt(index);
            dereferenceItem(item);
        }

        return true;
    }
    bool removeLast()
    {
        return remove(count()-1);
    }
    bool clear()
    {
        if(m_content.isEmpty())
            return true;

        QList<T*> tempList;
        for(const auto item: *this)
        {
            dereferenceItem(item);
            tempList.append(item);
        }
        m_content.clear();

        if(m_clearCallback)
            m_clearCallback();

        return true;
    }
    T* first() const
    {
        if(m_content.isEmpty())
        {
            QMODELSLOG_WARNING() << templateClassName() << "The first element of an empty list doesn't exist !";
            return nullptr;
        }

        return m_content.first();
    }
    T* last() const
    {
        if(m_content.isEmpty())
        {
            QMODELSLOG_WARNING() << templateClassName() << "The last element of an empty list doesn't exist !";
            return nullptr;
        }

        return m_content.last();
    }

    const QList<T*>& toList() const
    {
        return m_content;
    }

    // ──────── PUBLIC OBSERVER API ──────────
public:
    bool onInserted(std::function<void(T* object)> callback)
    {
        if(!callback)
        {
            QMODELSLOG_WARNING() << "onAdded: Fail to connect empty lambda";
            return false;
        }

        if(m_addCallback)
        {
            QMODELSLOG_WARNING() << "onAdded: can have only one callback";
            return false;
        }

        m_addCallback = callback;
        return true;
    }
    bool onRemoved(std::function<void(T* object)> callback)
    {
        if(!callback)
        {
            QMODELSLOG_WARNING() << "onRemoved: Fail to connect empty lambda";
            return false;
        }

        if(m_removeCallback)
        {
            QMODELSLOG_WARNING() << "onRemoved: can have only one callback";
            return false;
        }

        m_removeCallback = callback;
        return true;
    }
    bool onClear(std::function<void()> callback)
    {
        if(!callback)
        {
            QMODELSLOG_WARNING() << "onClear: Fail to connect empty lambda";
            return false;
        }

        if(m_clearCallback)
        {
            QMODELSLOG_WARNING() << "onClear: can have only one callback";
            return false;
        }

        m_clearCallback = callback;
        return true;
    }
    // ──────── PRIVATE LIST PROPERTY ──────────
private:
#if QT_VERSION_MAJOR < 6
    using qolp_size_type = int;
#else
    using qolp_size_type = qsizetype;
#endif
    static void list_append(QmlListProperty* self, T* object) {
        QObjectListProperty<T>* that = static_cast<QObjectListProperty<T>*>(self->data);
        if (that != Q_NULLPTR) {
            that->append(object);
        }
    };
    static qolp_size_type list_count(QmlListProperty* self) {
        QObjectListProperty<T>* that = static_cast<QObjectListProperty<T>*>(self->data);
        if (that != Q_NULLPTR) {
            return that->count();
        }
        return 0;
    };
    static T* list_at(QmlListProperty* self, qolp_size_type index) {
        QObjectListProperty<T>* that = static_cast<QObjectListProperty<T>*>(self->data);
        if (that != Q_NULLPTR) {
            return that->at(index);
        }
        return 0;
    };
    static void list_clear(QmlListProperty* self) {
        QObjectListProperty<T>* that = static_cast<QObjectListProperty<T>*>(self->data);
        if (that != Q_NULLPTR) {
            that->clear();
        }
    };
    static void list_replace(QmlListProperty* self, qolp_size_type index, T* object) {
        QObjectListProperty<T>* that = static_cast<QObjectListProperty<T>*>(self->data);
        if (that != Q_NULLPTR) {
            that->replace(index, object);
        }
    };
    static void list_removeLast(QmlListProperty* self) {
        QObjectListProperty<T>* that = static_cast<QObjectListProperty<T>*>(self->data);
        if (that != Q_NULLPTR) {
            that->removeLast();
        }
    };

    // ──────── ABSTRACT LIST PRIVATE ──────────
private:
    void referenceItem(T* item)
    {
        if(item != nullptr)
        {
            if(!item->parent())
            {
                item->setParent(this->object);
                QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
            }

            if(m_addCallback)
                m_addCallback(item);
        }
    }
    void dereferenceItem(T* item)
    {
        if(item != nullptr)
        {
            if(item->parent() == this->object)
            {
                item->deleteLater();
            }

            if(m_removeCallback)
                m_removeCallback(item);
        }
    }

private:
    QList<T*> m_content;

    std::function<void(T* object)> m_addCallback=nullptr;
    std::function<void(T* object)> m_removeCallback=nullptr;
    std::function<void()> m_clearCallback=nullptr;
};

#endif // QOBJECTLISTPROPERTY_H
