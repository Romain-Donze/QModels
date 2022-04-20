#ifndef QCONCATENATEPROXYMODEL_H
#define QCONCATENATEPROXYMODEL_H

#include <QtQml>
#include <QAbstractListModel>

class QConcatenateProxyModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ConcatenateProxyModel)
public:
    explicit QConcatenateProxyModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const final override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    void addSourceModel(QAbstractItemModel *sourceModel);
    void removeSourceModel(QAbstractItemModel *sourceModel);
    void clearSourceModel();

protected slots:
    void onDataChanged(const QModelIndex &from, const QModelIndex &to, const QVector<int> &roles);

private:
    QList<QAbstractItemModel*> m_sourceModels;
};

#endif // QCONCATENATEPROXYMODEL_H
