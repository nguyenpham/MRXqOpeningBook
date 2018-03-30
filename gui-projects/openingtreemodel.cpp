
#include <QtWidgets>

#include "treeitem.h"
#include "openingtreemodel.h"
#include "boardadapter.h"

OpeningTreeModel::OpeningTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new TreeItem(0, 0, 0, "");
}

OpeningTreeModel::~OpeningTreeModel()
{
    delete rootItem;
}

bool OpeningTreeModel::loadOpening(const QString& openingPath, int sd)
{
    if (sd != 0 && sd != 1) {
        return false;
    }
    QFileInfo check_file(openingPath);
    if (!check_file.exists() || !check_file.isFile()) {
        return false;
    }

    if (!m_opBook.load(openingPath.toStdString())) {
        qDebug() << "OpeningTreeModel::loadOpening load FAILED";
        return false;
    }

    beginResetModel();
    auto side = static_cast<opening::Side>(sd);
    m_showingSide = side;

    m_workingItems.clear();
    delete rootItem;

    rootItem = new TreeItem(0, 0, 0, "");

    opening::OpeningBoard board;
    board.setFen("");

    auto r = buildTree(rootItem, board, sd, 2);

    qDebug() << "OpeningTreeModel::loadOpening rootItem children cnt " << rootItem->childCount() << ", r = " << r;

//    emit dataChanged(QModelIndex(), QModelIndex());
//    layoutChanged();

    endResetModel();
    return true;
}

bool OpeningTreeModel::buildTree(TreeItem* treeItem, opening::OpeningBoard& board, int sd, int moreply)
{
    moreply--;
    if (moreply < 0) {
        return false;
    }

    auto side = board.side;
    auto sameSide = static_cast<int>(side) == sd;

    bool r = false;

    if (!sameSide) {
        auto idx = m_opBook.find(board.key(), sd);
        if (idx < 0) {
            return false;
        }

        treeItem->value = m_opBook.getValue(idx, sd);
        treeItem->prop |= TreeItem::Prop_hasValue;
        r = true;
    }

    opening::MoveList moveList;
    board.gen(moveList, board.side);

    for(int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        board.make(move);
        if (!board.isIncheck(side)) {
            auto child = new TreeItem(move.from, move.dest, 0, "");
            if (buildTree(child, board, sd, moreply)) {
                treeItem->addChild(child);
                treeItem->prop |= TreeItem::Prop_hasExpanded;
                r = true;
            } else {
                delete child;
            }
        }
        board.takeBack();
    }

    return r;
}


bool OpeningTreeModel::removeExpandedItems(const QModelIndex &index)
{
//    auto node = static_cast<TreeItem*>(index.internalPointer());
//    if (node) {
//        node->childItems.clear();
//        node->prop &= ~TreeItem::Prop_hasExpanded;
//        emit dataChanged(index, index);
//        return true;
//    }
    return false;
}

bool OpeningTreeModel::expand(const QModelIndex &index)
{
    beginInsertRows(index, 0, 0);

    auto node = static_cast<TreeItem*>(index.internalPointer());
    auto r = _expand(node);

    endInsertRows();

    emit dataChanged(index, index);
//    emit dataChanged(QModelIndex(), QModelIndex());
    layoutChanged();
    return r;
}

bool OpeningTreeModel::_expand(TreeItem* node)
{
    if (!node || (node->prop & TreeItem::Prop_hasExpanded)) {
        return false;
    }

    node->prop |= TreeItem::Prop_hasExpanded;

    std::vector<opening::Move> moves;
    for(auto nd = node; nd && nd->from != nd->dest; nd = nd->parent()) {
        opening::Move move(nd->from, nd->dest);
        moves.insert(moves.begin(), move);
//        qDebug() << "OpeningTreeModel::expand, nd " << nd->from << "->" << nd->dest << ", moves sz = " << moves.size();
    }

    opening::OpeningBoard board;
    board.setFen("");

    for(auto && move : moves) {
        board.make(move.from, move.dest);
    }

    auto sd = static_cast<int>(m_showingSide);
    buildTree(node, board, sd, 3);

    return true;
}

int OpeningTreeModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}


QHash<int, QByteArray> OpeningTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TreeModelRoleName] = "title";
    return roles;
}

QVariant OpeningTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != TreeModelRoleName)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    QString s = item->data();
    return QVariant(m_workingItems.contains(item) ? s + "@" : s);
}

QVariantList OpeningTreeModel::selectionChanged(const QModelIndex &theindex)
{
    QVariantList list;
    m_workingItems.clear();

    TreeItem *item = static_cast<TreeItem*>(theindex.internalPointer());

    for(auto nd = item; nd && nd->from != nd->dest; nd = nd->parent()) {
        m_workingItems << nd;
        int k = (int)nd->from | ((int)nd->dest) << 8;
        list.insert(0, qVariantFromValue(k));
    }

    return list;
}


Qt::ItemFlags OpeningTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *OpeningTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QModelIndex OpeningTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, 0, childItem);
    }
    return QModelIndex();
}

QModelIndex OpeningTreeModel::parent(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
        if (childItem != nullptr) {
            TreeItem *parentItem = childItem->parent();
            if (parentItem && parentItem != rootItem) {
                return createIndex(parentItem->getIndexRow(), 0, parentItem);
            }
        }
    }
    return QModelIndex();
}

int OpeningTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

bool OpeningTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    return true;
}
