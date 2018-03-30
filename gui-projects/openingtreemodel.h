#ifndef OPENINGTREEMODEL_H
#define OPENINGTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSet>

#include "../source/OpBook.h"

class TreeItem;
class BoardAdapter;

class OpeningTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit OpeningTreeModel(QObject *parent = 0);
    virtual ~OpeningTreeModel();

    enum TreeModelRoles
    {
        TreeModelRoleName = Qt::UserRole + 1
    };

    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariantList selectionChanged(const QModelIndex &index);

    Q_INVOKABLE bool loadOpening(const QString& openingPath, int sd);
    Q_INVOKABLE bool expand(const QModelIndex &index);
    Q_INVOKABLE bool removeExpandedItems(const QModelIndex &index);

private:
//    void updateIndexes();
//    int updateIndexes(TreeItem *item, int row, int col);
//    TreeItem* getBottomRightNode(TreeItem *item);

    bool _expand(TreeItem* node);
    bool buildTree(TreeItem* treeItem, opening::OpeningBoard& board, int sd, int ply);

    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;

    opening::OpBook m_opBook;

    QSet<TreeItem*> m_workingItems;

    opening::Side m_showingSide;
};

#endif // OPENINGTREEMODEL_H
