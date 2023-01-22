#include "qcsvlistmodel.h"
#include "qmodels_log.h"

QCsvListModel::QCsvListModel(QObject *parent) :
    QVariantListModel(parent)
{

}

bool QCsvListModel::loadPath(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMODELSLOG_WARNING()<<"Error opening file:"<<file.errorString();
        return false;
    }

    bool ret = loadCsv(file.readAll());
    file.close();

    return ret;
}

bool QCsvListModel::loadCsv(const QByteArray& csv)
{
    QTextStream fileStream(csv);

    QVariantList storage;

    do
    {
        QString line = fileStream.readLine();
        const QStringList values = line.split(m_separator);
        QVariantMap map;
        int pos=0;

        for(const QString& value: values)
        {
            map.insert(QString("column_%1").arg(pos), value);
            pos++;
        }

        storage.append(map);
    }
    while(!fileStream.atEnd());

    return setStorage(storage);
}

bool QCsvListModel::syncPath(const QString& fileName) const
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QMODELSLOG_WARNING()<<"cannot open file:"<<fileName;
        return false;
    }

    bool ret = file.write(toCsv());
    file.close();

    return ret;
}

QByteArray QCsvListModel::toCsv() const
{
    QByteArray csv;

    for(const QVariant& variant: storage())
    {
        QByteArray csvLine;
        const QVariantMap values = variant.toMap();
        for (QVariantMap::const_iterator it = values.begin(); it != values.end(); ++it)
        {
            csvLine.append(it.value().toString());
            csvLine.append(separator());
        }
        csvLine.chop(1);
        csvLine.append('\n');

        csv.append(csvLine);
    }

    return csv;
}

char QCsvListModel::separator() const
{
    return m_separator;
}

bool QCsvListModel::setSeparator(char separator)
{
    if(m_separator==separator)
        return false;
    m_separator=separator;
    emit this->separatorChanged(m_separator);
    return true;
}
