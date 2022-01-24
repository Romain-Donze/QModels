#ifndef QVARIANTLISTMODEL_H
#define QVARIANTLISTMODEL_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QList>
#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QDebug>

class QVariantListModel: public QAbstractListModel,
                         public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int length READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int size READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int isEmpty READ isEmpty NOTIFY emptyChanged FINAL)

    // ──────── CONSTRUCTOR ──────────
public:
    explicit QVariantListModel(QObject * parent = nullptr);

    static const QModelIndex& noParent();

    // ──────── ABSTRACT MODEL OVERRIDE ──────────
public:
    bool setData(const QModelIndex& modelIndex, const QVariant& value, int role) override final;
    QVariant data(const QModelIndex& modelIndex, int role) const override final;

    QHash<int, QByteArray> roleNames() const override final;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override final;

    // ──────── PUBLIC API ──────────
public:
    void classBegin() override {};
    void componentComplete() override {};

    int count() const { return rowCount(); };
    int size() const { return count(); };
    int length() const { return count(); };
    bool isEmpty() const { return count() == 0; };

    Q_INVOKABLE QVariant at(int index) const;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE bool append(const QVariant& variant);
    Q_INVOKABLE bool prepend(const QVariant& variant);
    Q_INVOKABLE bool insert(int index, const QVariant& variant);
    Q_INVOKABLE bool replace(int index, const QVariant& variant);
    Q_INVOKABLE bool remove(int index, int count=1);
    Q_INVOKABLE bool clear();

signals:
    void countChanged(int count);
    void emptyChanged(bool empty);

    // ──────── ABSTRACT MODEL PRIVATE ──────────
protected:
    void countInvalidate();

    // ──────── ITERATOR ──────────
public:
    using const_iterator = typename QList<QVariant>::const_iterator;
    const_iterator begin() const { return m_variants.begin(); }
    const_iterator end() const { return m_variants.end(); }
    const_iterator cbegin() const { return m_variants.begin(); }
    const_iterator cend() const { return m_variants.end(); }
    const_iterator constBegin() const { return m_variants.constBegin(); }
    const_iterator constEnd() const { return m_variants.constEnd(); }

    using const_reverse_iterator = typename QList<QVariant>::const_reverse_iterator;
    const_reverse_iterator rbegin() const { return m_variants.rbegin(); }
    const_reverse_iterator rend() const { return m_variants.rend(); }
    const_reverse_iterator crbegin() const { return m_variants.crbegin(); }
    const_reverse_iterator crend() const { return m_variants.crend(); }

    // ──────── ATTRIBUTES ──────────
private:
    int m_count=0;
    QList<QVariant> m_variants;
};

#endif // QVARIANTLISTMODEL_H
