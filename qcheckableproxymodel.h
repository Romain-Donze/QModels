#ifndef QCHECKABLEPROXYMODEL_H
#define QCHECKABLEPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QItemSelection>
#include <QtQml>

class QCheckableProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CheckableProxyModel)
    Q_PROPERTY(QItemSelectionModel* selection READ getSelection CONSTANT)

public:
    explicit QCheckableProxyModel(QObject * parent = nullptr);

    bool setData(const QModelIndex& modelIndex, const QVariant& value, int role) override;
    QVariant data(const QModelIndex& modelIndex, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QItemSelectionModel *getSelection() const;

    Q_INVOKABLE QList<int> selectedRows() const;
    Q_INVOKABLE bool isChecked(int index) const;

public slots:
    void clear();
    void toggle(int index);

protected slots:
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void resetInternalData();
    void updateRoleNames();
    void initRoles();

private:
    QItemSelectionModel *m_selection = nullptr;
    QHash<int, QByteArray> m_roleNames;
};

#endif // QCHECKABLEPROXYMODEL_H
