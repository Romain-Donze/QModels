#ifndef QOBJECTLISTMODELBASE_H
#define QOBJECTLISTMODELBASE_H

#include <QAbstractListModel>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QtCore/QAbstractListModel>
#include <QtQml/QQmlListProperty>
#include <QtQml/QJSValue>

class QObjectListModelBase : public QAbstractListModel,
                             public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int length READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int size READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int isEmpty READ isEmpty NOTIFY emptyChanged FINAL)
    Q_PROPERTY(QQmlListProperty<QObject> content READ getContent CONSTANT FINAL)
    Q_CLASSINFO("DefaultProperty", "content")

    // ──────── CONSTRUCTOR ──────────
public:
    explicit QObjectListModelBase(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    {
        QObject::connect(this, &QAbstractItemModel::rowsInserted, this, &QObjectListModelBase::countInvalidate);
        QObject::connect(this, &QAbstractItemModel::rowsRemoved, this, &QObjectListModelBase::countInvalidate);
        QObject::connect(this, &QAbstractItemModel::modelReset, this, &QObjectListModelBase::countInvalidate);
        QObject::connect(this, &QAbstractItemModel::layoutChanged, this, &QObjectListModelBase::countInvalidate);
    }

    void classBegin() override {};
    void componentComplete() override {};

    int count() const { return rowCount(); };
    int size() const { return count(); };
    int length() const { return count(); };
    bool isEmpty() const { return count() == 0; };

    Q_INVOKABLE virtual QObject* at(QJSValue index) const = 0;
    Q_INVOKABLE virtual QObject* get(QJSValue index) const = 0;
    Q_INVOKABLE virtual int indexOf(QJSValue object) const = 0;
    Q_INVOKABLE virtual bool contains(QJSValue object) const = 0;
    Q_INVOKABLE virtual int roleForName(const QByteArray& name) const = 0;
    Q_INVOKABLE virtual QByteArray roleName(int role) const = 0;

public slots:
    virtual bool append(QJSValue object) = 0;
    virtual bool prepend(QJSValue item) = 0;
    virtual bool insert(int index, QJSValue item) = 0;
    virtual bool remove(QJSValue value) = 0;
    virtual bool clear(void) = 0;

    virtual bool move(int index, int to) = 0;
    virtual bool moveDown(const int index) = 0;
    virtual bool moveUp(const int index) = 0;

    bool moveNext(const int index) { return moveDown(index); };
    bool movePrevious(const int index) { return moveUp(index); };

signals:
    void countChanged(int count);
    void emptyChanged(bool empty);

    void objectInserted(QObject* object, int index);
    void objectRemoved(QObject* object, int index);
    void objectMoved(QObject* object, int from, int to);

protected slots:
    virtual void onItemPropertyChanged() = 0;
    virtual void countInvalidate() = 0;

    // ──────── DEFAULT QML CONTENT ──────────
public:
    QQmlListProperty<QObject> getContent()
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        return QQmlListProperty<QObject>(this, this,
                                         &QObjectModelBase::content_append,
                                         &QObjectModelBase::content_count,
                                         &QObjectModelBase::content_at,
                                         &QObjectModelBase::content_clear);
#else
        return QQmlListProperty<QObject>(this, this,
                                         &QObjectListModelBase::content_append,
                                         &QObjectListModelBase::content_count,
                                         &QObjectListModelBase::content_at,
                                         &QObjectListModelBase::content_clear,
                                         &QObjectListModelBase::content_replace,
                                         &QObjectListModelBase::content_removeLast);
#endif
    }

protected:
    virtual void content_append(QObject* child) = 0;
    virtual int content_count() = 0;
    virtual QObject* content_at(int index) = 0;
    virtual void content_clear() = 0;
    virtual void content_replace(int index, QObject* child) = 0;
    virtual void content_removeLast() = 0;

private:
    static void content_append(QQmlListProperty<QObject>* list, QObject* child)
    {
        QObjectListModelBase* that = reinterpret_cast<QObjectListModelBase*>(list->object);
        return that->content_append(child);
    }
    static int content_count(QQmlListProperty<QObject>* list)
    {
        QObjectListModelBase* that = reinterpret_cast<QObjectListModelBase*>(list->object);
        return that->content_count();
    }
    static QObject* content_at(QQmlListProperty<QObject>* list, int index)
    {
        QObjectListModelBase* that = reinterpret_cast<QObjectListModelBase*>(list->object);
        return that->content_at(index);
    }
    static void content_clear(QQmlListProperty<QObject>* list)
    {
        QObjectListModelBase* that = reinterpret_cast<QObjectListModelBase*>(list->object);
        return that->content_clear();
    }
    static void content_replace(QQmlListProperty<QObject>* list, int index, QObject* child)
    {
        QObjectListModelBase* that = reinterpret_cast<QObjectListModelBase*>(list->object);
        return that->content_replace(index, child);
    }
    static void content_removeLast(QQmlListProperty<QObject>* list)
    {
        QObjectListModelBase* that = reinterpret_cast<QObjectListModelBase*>(list->object);
        return that->content_removeLast();
    }
};

#endif
