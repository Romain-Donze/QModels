#ifndef QEMPTYMODEL_H
#define QEMPTYMODEL_H

#include <QAbstractItemModel>

class QEmptyModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QEmptyModel(QObject *parent = 0) : QAbstractItemModel(parent) {}
    QModelIndex index(int, int, const QModelIndex &) const override final { return QModelIndex(); }
    QModelIndex parent(const QModelIndex &) const override final { return QModelIndex(); }
    int rowCount(const QModelIndex &) const override final { return 0; }
    int columnCount(const QModelIndex &) const override final { return 0; }
    bool hasChildren(const QModelIndex &) const override final { return false; }
    QVariant data(const QModelIndex &, int) const override final { return QVariant(); }
};


#endif // QEMPTYMODEL_H
