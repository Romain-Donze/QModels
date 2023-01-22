#ifndef QJSONTREEMODEL_H
#define QJSONTREEMODEL_H

#include <QAbstractItemModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QtQml>

class QJsonTreeItem
{
public:
    QJsonTreeItem(QJsonTreeItem *parent = nullptr)  { m_parentItem = parent; };
    ~QJsonTreeItem()                                { qDeleteAll(m_childs); };

    void appendChild(QJsonTreeItem *item)           { m_childs.append(item); };
    QJsonTreeItem *child(int row)                   { return m_childs.value(row); };
    QJsonTreeItem *parentItem()                     { return m_parentItem; };
    int childCount() const                          { return m_childs.count(); };
    int row() const                                 { return m_parentItem ? m_parentItem->m_childs.indexOf(const_cast<QJsonTreeItem*>(this)) : 0; };

    void setKey(const QString& key)                 { m_key = key; };
    void setValue(const QVariant& value)            { m_value = value; };
    void setType(const QJsonValue::Type& type)      { m_type = type; };
    QString key() const                             { return m_key; };
    QVariant value() const                          { return m_value; };
    QJsonValue::Type type() const                   { return m_type; };

    static QJsonTreeItem* load(const QJsonValue& value, QJsonTreeItem *parent = nullptr);

private:
    QString m_key;
    QVariant m_value;
    QJsonValue::Type m_type;
    QList<QJsonTreeItem*> m_childs;
    QJsonTreeItem *m_parentItem = nullptr;
};

class QJsonTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(JsonTreeModel)

public:
    explicit QJsonTreeModel(QObject *parent = nullptr);
    QJsonTreeModel(const QString& fileName, QObject *parent = nullptr);
    QJsonTreeModel(QIODevice *device, QObject *parent = nullptr);
    QJsonTreeModel(const QByteArray& json, QObject *parent = nullptr);
    ~QJsonTreeModel();

    bool loadPath(const QString& fileName);
    bool loadFile(QIODevice *device);
    bool loadJson(const QByteArray& json);

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QByteArray json();
    QByteArray jsonToByte(QJsonValue jsonValue);
    void objectToJson(QJsonObject jsonObject, QByteArray &json, int indent, bool compact);
    void arrayToJson(QJsonArray jsonArray, QByteArray &json, int indent, bool compact);
    void arrayContentToJson(QJsonArray jsonArray, QByteArray &json, int indent, bool compact);
    void objectContentToJson(QJsonObject jsonObject, QByteArray &json, int indent, bool compact);
    void valueToJson(QJsonValue jsonValue, QByteArray &json, int indent, bool compact);

private:
    QJsonValue genJson(QJsonTreeItem *) const;
    QJsonTreeItem *m_rootItem = nullptr;
    QStringList m_headers;
};

#endif // QJSONTREEMODEL_H
