#ifndef QOBJECTLISTPROPERTY_H
#define QOBJECTLISTPROPERTY_H

#include <QQmlListProperty>

template<class T>
class QObjectListProperty : public QQmlListProperty<T>
{
    // ──────── CONSTRUCTOR ──────────
public:
    using QmlListProperty = QQmlListProperty<T>;

    explicit QObjectListProperty(QObject* object) :
        QmlListProperty(
              object,
              &m_content,
              &QObjectListProperty<T>::list_append,
              &QObjectListProperty<T>::list_count,
              &QObjectListProperty<T>::list_at,
              &QObjectListProperty<T>::list_clear,
              &QObjectListProperty<T>::list_replace,
              &QObjectListProperty<T>::list_removeLast)
    { }

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

    T* at(int index) const { return m_content.at(index); }
    T* get(int index) const { return m_content.get(index); }

    bool contains(const T* object) const { return m_content.contains(object); }
    int indexOf(const T* object) const { return m_content.indexOf(object); }
    bool append(T* object) { return m_content.append(object); }
    bool prepend(T* object) { return m_content.prepend(object); }
    bool insert(int index, T* object) { return m_content.insert(index, object); }
    bool append(const QList<T*>& objectList) { return m_content.append(objectList); }
    bool prepend(const QList<T*>& objectList) { return m_content.prepend(objectList); }
    bool insert(int idx, const QList<T*>& itemList) { return m_content.insert(idx, itemList); }
    bool move(int from, int to) { return m_content.move(from, to); }
    bool remove(const T* object) { return m_content.remove(object); }
    bool remove(const QList<T*>& objects) { return m_content.remove(objects); }
    bool remove(int index, int count = 1) { return m_content.remove(index, count); }
    bool clear() { return m_content.clear(); }
    T* first() const { return m_content.first(); }
    T* last() const { return m_content.last(); }

    const QList<T*>& toList() const { return m_content; }

private:
#if QT_VERSION_MAJOR < 6
    using qolp_size_type = int;
#else
    using qolp_size_type = qsizetype;
#endif
    static void list_append(QmlListProperty* self, T* object) {
        QList<T *> * content = static_cast<QVector<T *> *> (self->data);
        if (content != Q_NULLPTR) {
            content->append (object);
        }
    };
    static qolp_size_type list_count(QmlListProperty* self) {
        QList<T*>* content = static_cast<QVector<T*>*>(self->data);
        if (content != Q_NULLPTR) {
            return content->count();
        }
        return 0;
    };
    static T* list_at(QmlListProperty* self, qolp_size_type index) {
        QList<T*>* content = static_cast<QVector<T*>*>(self->data);
        if (content != Q_NULLPTR) {
            return content->at(index);
        }
        return 0;
    };
    static void list_clear(QmlListProperty* self) {
        QList<T*>* content = static_cast<QVector<T*>*>(self->data);
        if (content != Q_NULLPTR) {
            return content->clear();
        }
    };
    static void list_replace(QmlListProperty* self, qolp_size_type index, T* object) {
        QList<T*>* content = static_cast<QVector<T*>*>(self->data);
        if (content != Q_NULLPTR) {
            return content->replace(index, object);
        }
    };
    static void list_removeLast(QmlListProperty* self) {
        QList<T*>* content = static_cast<QVector<T*>*>(self->data);
        if (content != Q_NULLPTR) {
            return content->removeLast();
        }
    };

private:
    QList<T*> m_content;
};

#endif // QOBJECTLISTPROPERTY_H
