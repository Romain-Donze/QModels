#ifndef QJSONLISTMODEL_H
#define QJSONLISTMODEL_H

#include "qvariantlistmodel.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>

class QJsonListModel: public QVariantListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(JsonListModel)

public:
    explicit QJsonListModel(QObject * parent = nullptr);

    enum JsonFormat {
        Indented = QJsonDocument::Indented,
        Compact = QJsonDocument::Compact
    };
    Q_ENUM (JsonFormat)

    Q_INVOKABLE bool syncPath(const QString& path) const;
    Q_INVOKABLE QByteArray toJson(QJsonListModel::JsonFormat format = Indented) const;

public slots:
    bool loadPath(const QString& path);
    bool loadJson(const QByteArray& json);
};

Q_DECLARE_METATYPE (QJsonListModel::JsonFormat)

#endif // QJSONLISTMODEL_H
