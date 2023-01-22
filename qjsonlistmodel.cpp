#include "qjsonlistmodel.h"
#include "qmodels_log.h"

QJsonListModel::QJsonListModel(QObject *parent) :
    QVariantListModel(parent)
{

}

bool QJsonListModel::loadPath(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMODELSLOG_WARNING()<<"Error opening file:"<<file.errorString();
        return false;
    }

    bool ret = loadJson(file.readAll());
    file.close();

    return ret;
}

bool QJsonListModel::loadJson(const QByteArray& json)
{
    QJsonParseError parseError;
    const QJsonDocument& jdoc = QJsonDocument::fromJson(json, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        QMODELSLOG_WARNING()<<"Error loading json:"<<parseError.errorString();
        return false;
    }

    if (jdoc.isNull())
    {
        QMODELSLOG_WARNING()<<"cannot load json";
        return false;
    }

    if(!jdoc.isArray())
    {
        QMODELSLOG_WARNING()<<"cannot load non array json";
        return false;
    }

    return setStorage(jdoc.array().toVariantList());
}

bool QJsonListModel::syncPath(const QString& fileName) const
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QMODELSLOG_WARNING()<<"cannot open file:"<<fileName;
        return false;
    }

    bool ret = file.write(toJson());
    file.close();

    return ret;
}

QByteArray QJsonListModel::toJson(QJsonListModel::JsonFormat format) const
{
    return QJsonDocument(QJsonArray::fromVariantList(storage())).toJson(QJsonDocument::JsonFormat(format));
}
