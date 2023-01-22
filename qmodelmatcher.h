#ifndef QMODELMATCHER_H
#define QMODELMATCHER_H

#include <QObject>
#include <QAbstractItemModel>
#include <QQmlParserStatus>
#include <QQmlEngine>

class QModelMatcher : public QObject,
                      public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ModelMatcher)
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int length READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int size READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY emptyChanged FINAL)

    Q_PROPERTY(QModelIndexList indexes READ getIndexes NOTIFY indexesChanged)
    Q_PROPERTY(QAbstractItemModel *sourceModel READ getSourceModel WRITE setSourceModel NOTIFY sourceModelChanged)

    Q_PROPERTY(bool delayed READ getDelayed WRITE setDelayed NOTIFY delayedChanged)
    Q_PROPERTY(int startRow READ getStartRow WRITE setStartRow NOTIFY startRowChanged)
    Q_PROPERTY(int startColumn READ getStartColumn WRITE setStartColumn NOTIFY startColumnChanged)
    Q_PROPERTY(int role READ getRole WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(QString roleName READ getRoleName WRITE setRoleName NOTIFY roleNameChanged)
    Q_PROPERTY(QVariant value READ getValue WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int hits READ getHits WRITE setHits NOTIFY hitsChanged)
    Q_PROPERTY(Qt::MatchFlags flags READ getFlags WRITE setFlags NOTIFY flagsChanged)

public:
    explicit QModelMatcher(QObject* parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

    int count() const { return m_indexes.count(); };
    int size() const { return count(); };
    int length() const { return count(); };
    bool isEmpty() const { return count() == 0; };

    const QModelIndexList &getIndexes() const;
    QAbstractItemModel *getSourceModel() const;

    bool getDelayed() const;
    int getStartRow() const;
    int getStartColumn() const;
    int getRole() const;
    const QString &getRoleName() const;
    const QVariant &getValue() const;
    int getHits() const;
    const Qt::MatchFlags &getFlags() const;

public slots:
    virtual void setSourceModel(QAbstractItemModel *sourceModel);

    void setDelayed(bool delayed);
    void setStartRow(int startRow);
    void setStartColumn(int startColumn);
    void setRole(int role);
    void setRoleName(const QString &roleName);
    void setValue(const QVariant &value);
    void setHits(int hits);
    void setFlags(const Qt::MatchFlags &flags);

signals:
    void aboutToBeInvalidated();
    void invalidated();

    void countChanged();
    void emptyChanged();

    void indexesChanged(const QModelIndexList &indexes);

    void sourceModelChanged(QAbstractItemModel *sourceModel);
    void delayedChanged(bool delayed);
    void startRowChanged(int startRow);
    void startColumnChanged(int startColumn);
    void roleChanged(int role);
    void roleNameChanged(const QString &roleName);
    void valueChanged(const QVariant &value);
    void hitsChanged(int hits);
    void flagsChanged(const Qt::MatchFlags &flags);

protected slots:
    void countInvalidate();

    virtual void queueInvalidate();
    virtual void invalidate();
    void updateFilterRole();
    void updateRoles();
    void initRoles();

private:
    QModelIndexList m_indexes;

    int m_count=0;
    bool m_delayed=false;
    bool m_invalidateQueued = false;

    QAbstractItemModel* m_sourceModel=nullptr;
    int m_startRow=0;
    int m_startColumn=0;
    int m_role=Qt::DisplayRole;
    QString m_roleName="";
    QVariant m_value=QVariant();
    int m_hits=-1;
    Qt::MatchFlags m_flags=Qt::MatchExactly;
};

#endif // QMODELMATCHER_H
