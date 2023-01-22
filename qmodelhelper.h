#ifndef QMODELHELPER_H
#define QMODELHELPER_H

#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QQmlPropertyMap>
#include <QQmlEngine>

class QModelHelper : public QAbstractItemModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ModelHelper)
    QML_UNCREATABLE("Attached")

    // ──────── CONSTRUCTOR ──────────
private:
    explicit QModelHelper(QAbstractItemModel* object);

public:
    static QModelHelper* wrap(QObject* object);
    static QModelHelper* qmlAttachedProperties(QObject* object);

    // ──────── ABSTRACT MODEL OVERRIDE ──────────
public:
    bool setData(const QModelIndex& modelIndex, const QVariant& value, int role=Qt::EditRole) override final;
    QVariant data(const QModelIndex& modelIndex, int role=Qt::DisplayRole) const override final;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override final;
    QModelIndex parent(const QModelIndex &child) const override final;

    QHash<int, QByteArray> roleNames() const override final;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override final;

    Q_INVOKABLE int roleForName(const QByteArray& name) const;
    Q_INVOKABLE QByteArray roleName(int role) const;

    // ──────── COUNT PUBLIC API ──────────
private:
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int length READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int size READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY emptyChanged FINAL)

protected slots:
    void countInvalidate();

signals:
    void countChanged(int count);
    void emptyChanged(bool empty);

    // ──────── MAP PUBLIC API ──────────
public:
    Q_INVOKABLE QQmlPropertyMap* map(int row, int column = 0, const QModelIndex& parent = {});

    // ──────── PUBLIC API ──────────
public:
    int count() const { return rowCount(); };
    int size() const { return count(); };
    int length() const { return count(); };
    bool isEmpty() const { return count() == 0; };

    Q_INVOKABLE bool set(int row, const QVariantMap& pArray);
    Q_INVOKABLE bool setProperty(int row, const QString& property, const QVariant& value);
    Q_INVOKABLE bool setProperties(const QString& property, const QVariant& value);
    Q_INVOKABLE bool setProperties(const QList<int>& indexes, const QString& property, const QVariant& value);
    Q_INVOKABLE QVariantMap get(int row, const QStringList roles = QStringList()) const;
    Q_INVOKABLE QVariant getProperty(int row, const QString& property) const;
    Q_INVOKABLE QVariantList getProperties(const QString& property) const;
    Q_INVOKABLE QVariantList getProperties(const QList<int>& indexes, const QString& property) const;

    Q_INVOKABLE int indexOf(const QString &columnName, const QVariant& val, bool isSorted=false) const;
    Q_INVOKABLE QList<int> indexesOf(const QString &columnName, const QVariant& val) const;
    Q_INVOKABLE int count(const QString &columnName, const QVariant& val) const;
    Q_INVOKABLE bool contains(const QString& columnName, const QVariant& val, bool isSorted=false) const;
    Q_INVOKABLE bool isSorted(const QString& columnName) const;

    Q_INVOKABLE bool equals(QAbstractItemModel* model) const;
    Q_INVOKABLE QVariantList toVariantList() const;

    // TODO: backup return QAbstractItemModel*
    Q_INVOKABLE const QVariantList& backup() const;
    Q_INVOKABLE bool clearBackup();
    Q_INVOKABLE bool hasChanged() const;

    // ──────── PUBLIC STATIC API ──────────
public:
    static int roleForName(QAbstractItemModel* model, const QByteArray& name);
    static QByteArray roleName(QAbstractItemModel* model, int role);

    static int count(QAbstractItemModel* model) { return model->rowCount(); };
    static int size(QAbstractItemModel* model) { return count(model); };
    static int length(QAbstractItemModel* model) { return count(model); };
    static bool isEmpty(QAbstractItemModel* model) { return count(model) == 0; };

    static bool set(QAbstractItemModel* model, int row, const QVariantMap& pArray);
    static bool setProperty(QAbstractItemModel* model, int row, const QString& property, const QVariant& value);
    static bool setProperties(QAbstractItemModel* model, const QString& property, const QVariant& value);
    static bool setProperties(QAbstractItemModel* model, const QList<int>& indexes, const QString& property, const QVariant& value);
    static QVariantMap get(QAbstractItemModel* model, int row, const QStringList roles = QStringList());
    static QVariant getProperty(QAbstractItemModel* model, int row, const QString& property);
    static QVariantList getProperties(QAbstractItemModel* model, const QString& property);
    static QVariantList getProperties(QAbstractItemModel* model, const QList<int>& indexes, const QString& property);

    static int indexOf(QAbstractItemModel* model, int role, const QVariant& val, bool isSorted=false);
    static QList<int> indexesOf(QAbstractItemModel* model, int role, const QVariant& val);
    static int count(QAbstractItemModel* model, int role, const QVariant& val);
    static bool contains(QAbstractItemModel* model, int role, const QVariant& val, bool isSorted=false);
    static bool isSorted(QAbstractItemModel* model, int role);

    static int indexOf(QAbstractItemModel* model, const QString &columnName, const QVariant& val, bool isSorted=false);
    static QList<int> indexesOf(QAbstractItemModel* model, const QString &columnName, const QVariant& val);
    static int count(QAbstractItemModel* model, const QString &columnName, const QVariant& val);
    static bool contains(QAbstractItemModel* model, const QString& columnName, const QVariant& val, bool isSorted=false);
    static bool isSorted(QAbstractItemModel* model, const QString& columnName);

    static bool equals(QAbstractItemModel* srcModel, QAbstractItemModel* compModel);
    static QVariantList toVariantList(QAbstractItemModel* model);

    // ──────── HELPER PRIVATE ──────────
private:
    QQmlPropertyMap* mapperForRow(int row) const;
    void removeMapper(QObject* mapper);

    // ──────── ATTRIBUTES ──────────
private:
    int m_count=0;
    QAbstractItemModel* m_sourceModel = nullptr;
    QVector<QPair<int, QQmlPropertyMap*>> m_mappers;

    QStandardItemModel* m_backupModel=nullptr;
    mutable QVariantList m_backup;
};

QML_DECLARE_TYPEINFO(QModelHelper, QML_HAS_ATTACHED_PROPERTIES)

#endif // QMODELHELPER_H
