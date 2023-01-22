#ifndef QVARIANTLISTMODEL_H
#define QVARIANTLISTMODEL_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QList>
#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QtQml>
#include <QDebug>

class QVariantListModel: public QAbstractListModel,
                         public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(VariantListModel)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int length READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int size READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int isEmpty READ isEmpty NOTIFY emptyChanged FINAL)

    Q_PROPERTY(QStringList fields READ fields WRITE setFields NOTIFY fieldsChanged)
    Q_PROPERTY(QVariantList storage READ storage WRITE setStorage NOTIFY storageChanged)

    // ──────── CONSTRUCTOR ──────────
public:
    explicit QVariantListModel(QObject * parent = nullptr);

    static const QModelIndex& noParent();

    // ──────── ABSTRACT MODEL OVERRIDE ──────────
public:
    QVariant data(const QModelIndex& modelIndex, int role) const override final;
    bool setData(const QModelIndex& modelIndex, const QVariant& value, int role) override final;

    QHash<int, QByteArray> roleNames() const override final;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override final;

    // ──────── PUBLIC API ──────────
public:
    void classBegin() override {};
    void componentComplete() override {};

    int count() const { return rowCount(); };
    int size() const { return count(); };
    int length() const { return count(); };
    bool isEmpty() const { return count() == 0; };

    const QStringList& fields() const;
    bool setFields(const QStringList& fields);

    Q_INVOKABLE bool append(const QJSValue& value);
    Q_INVOKABLE bool prepend(const QJSValue& value);
    Q_INVOKABLE bool insert(int index, const QJSValue& value);
    bool append(const QVariant& variant);
    bool prepend(const QVariant& variant);
    bool insert(int index, const QVariant& variant);

    Q_INVOKABLE QVariant at(int index) const;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE bool move(int from, int to, int count = 1);
    Q_INVOKABLE bool remove(int index, int count=1);
    Q_INVOKABLE bool remove(QList<int> indexes);
    Q_INVOKABLE bool clear();

    const QVariantList& storage() const;
    bool setStorage(const QVariant& storage);

public slots:
    bool setSource(QAbstractItemModel* model);

signals:
    void countChanged(int count);
    void emptyChanged(bool empty);
    void fieldsChanged(const QStringList& value);
    void storageChanged(const QVariantList& storage);

    // ──────── ABSTRACT MODEL PRIVATE ──────────
protected:
    void countInvalidate();
    void contentInvalidate();
    void updateRoleNames(const QVariant& var);

    // ──────── ITERATOR ──────────
public:
    using const_iterator = typename QVariantList::const_iterator;
    const_iterator begin() const { return m_storage.begin(); }
    const_iterator end() const { return m_storage.end(); }
    const_iterator cbegin() const { return m_storage.begin(); }
    const_iterator cend() const { return m_storage.end(); }
    const_iterator constBegin() const { return m_storage.constBegin(); }
    const_iterator constEnd() const { return m_storage.constEnd(); }

    using const_reverse_iterator = typename QVariantList::const_reverse_iterator;
    const_reverse_iterator rbegin() const { return m_storage.rbegin(); }
    const_reverse_iterator rend() const { return m_storage.rend(); }
    const_reverse_iterator crbegin() const { return m_storage.crbegin(); }
    const_reverse_iterator crend() const { return m_storage.crend(); }

    // ──────── ATTRIBUTES ──────────
protected:
    bool m_canWrite;

private:
    QStringList m_fields;
    QVariantList m_storage;

    int m_count=0;
    QHash<int, QByteArray> m_roleNames;
};

#endif // QVARIANTLISTMODEL_H
