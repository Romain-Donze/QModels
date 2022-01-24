#ifndef QMODELHELPER_H
#define QMODELHELPER_H

#include <QAbstractItemModel>
#include <QQmlPropertyMap>
#include <QQmlEngine>

class QModelHelperFilter;
class QModelHelper : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int length READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int size READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY emptyChanged FINAL)

    // ──────── CONSTRUCTOR ──────────
private:
    explicit QModelHelper(QAbstractItemModel* object);

public:
    static QModelHelper* wrap(QObject* object);
    static QModelHelper* qmlAttachedProperties(QObject* object);

    // ──────── ABSTRACT MODEL OVERRIDE ──────────
public:
    bool setData(const QModelIndex& modelIndex, const QVariant& value, int role) override final;
    QVariant data(const QModelIndex& modelIndex, int role) const override final;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override final;
    QModelIndex parent(const QModelIndex &child) const override final;

    QHash<int, QByteArray> roleNames() const override final;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override final;

    Q_INVOKABLE int roleForName(const QByteArray& name) const;
    Q_INVOKABLE QByteArray roleName(int role) const;

    // ──────── ABSTRACT MODEL PRIVATE ──────────
protected slots:
    void countInvalidate();

    // ──────── PUBLIC API ──────────
public:
    int count() const { return rowCount(); };
    int size() const { return count(); };
    int length() const { return count(); };
    bool isEmpty() const { return count() == 0; };

    Q_INVOKABLE QQmlPropertyMap* map(int row, int column = 0, const QModelIndex& parent = {});

    Q_INVOKABLE bool updateWhere(const QString& columnName, const QVariant& where, const QString& property, const QVariant& value);
    Q_INVOKABLE bool updateAll(const QString& property, const QVariant& value);
    Q_INVOKABLE bool set(int pIndex, const QVariantMap& pArray);
    Q_INVOKABLE bool setProperty(int pIndex, const QString& property, const QVariant& value);
    Q_INVOKABLE QVariantMap get(int row, const QStringList roles = QStringList()) const;
    Q_INVOKABLE QVariant getProperty(int pIndex, const QString& property) const;

    Q_INVOKABLE int indexOf(const QString &columnName, const QVariant& val) const;
    Q_INVOKABLE bool contains(const QString& columnName, const QVariant& val) const;

signals:
    void countChanged(int count);
    void emptyChanged(bool empty);

    // ──────── HELPER PRIVATE ──────────
private:
    QQmlPropertyMap* mapperForRow(int row) const;
    void removeMapper(QObject* mapper);

    // ──────── ATTRIBUTES ──────────
private:
    int m_count=0;
    QAbstractItemModel* m_sourceModel = nullptr;
    QVector<QPair<int, QQmlPropertyMap*>> m_mappers;

    mutable QModelHelperFilter* m_proxyModelPrivate;
};

QML_DECLARE_TYPEINFO(QModelHelper, QML_HAS_ATTACHED_PROPERTIES)

#endif // QMODELHELPER_H