#ifndef QCSVLISTMODEL_H
#define QCSVLISTMODEL_H

#include "qvariantlistmodel.h"

class QCsvListModel: public QVariantListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CsvListModel)

    Q_PROPERTY(char separator READ separator WRITE setSeparator NOTIFY separatorChanged)

public:
    explicit QCsvListModel(QObject * parent = nullptr);

    char separator() const;
    bool setSeparator(char separator);

    Q_INVOKABLE bool syncPath(const QString& path) const;
    Q_INVOKABLE QByteArray toCsv() const;

public slots:
    bool loadPath(const QString& path);
    bool loadCsv(const QByteArray& csv);

signals:
    void separatorChanged(char separator);

private:
    char m_separator=';';
};

#endif // QCSVLISTMODEL_H
