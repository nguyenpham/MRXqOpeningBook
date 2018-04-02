
/*
 This file is part of MoonRiver Xiangqi Opening Book, distributed under MIT license.

 Copyright (c) 2018 Nguyen Hong Pham

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <QtWidgets>

#include "treeitem.h"
#include "openingtreemodel.h"
#include "boardadapter.h"

OpeningTreeModel::OpeningTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new TreeItem();
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

    rootItem = new TreeItem();

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
//        auto idx = m_opBook.find(board.key(), sd);
//        if (idx < 0) {
//            return false;
//        }

//        treeItem->value = m_opBook.getValue(idx, sd);

        auto val = m_opBook.getValueByKeyFromMainData(board.key(), sd);
        if (val < 0) {
            return false;
        }

        treeItem->value = val;
        treeItem->learntValue = m_opBook.getValueByKeyFromLearntData(board.key(), sd);;

        treeItem->prop |= TreeItem::Prop_hasValue;
        r = true;
    }

    opening::MoveList moveList;
    board.gen(moveList, board.side);

    for(int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        board.make(move);
        if (!board.isIncheck(side)) {
            auto child = new TreeItem(move.from, move.dest);
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

//    std::vector<opening::MoveCore> moves;
//    for(auto nd = node; nd && nd->from != nd->dest; nd = nd->parent()) {
//        opening::MoveCore move(nd->from, nd->dest);
//        moves.insert(moves.begin(), move);
//    }

//    opening::OpeningBoard board;
//    board.setFen("");

//    for(auto && move : moves) {
//        board.make(move.from, move.dest);
//    }

    std::vector<opening::MoveCore> moves;
    opening::OpeningBoard board;
    getLine(node, moves, board);

    auto sd = static_cast<int>(m_showingSide);
    buildTree(node, board, sd, 3);

    return true;
}

void OpeningTreeModel::getLine(const TreeItem* node, std::vector<opening::MoveCore>& moves, opening::OpeningBoard& board)
{
    moves.clear();
    for(const TreeItem* nd = node; nd && nd->from != nd->dest; nd = nd->parent()) {
        opening::MoveCore move(nd->from, nd->dest);
        moves.insert(moves.begin(), move);
    }

    board.setFen("");

    for(auto && move : moves) {
        board.make(move.from, move.dest);
    }
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
    qDebug() << "OpeningTreeModel::setData, index = " << index << ", value = " << value;
    return true;
}

void OpeningTreeModel::updateValue(const QModelIndex &index, const QVariant &value, int saveTo) {
    qDebug() << "OpeningTreeModel::updateValue, index = " << index << ", value = " << value << ", saveTo = " << saveTo;

    if (!index.isValid()) {
        return;
    }

    auto node = static_cast<TreeItem*>(index.internalPointer());
    if (!node || node == rootItem || !(node->prop & TreeItem::Prop_hasValue)) {
        return;
    }

    std::vector<opening::MoveCore> moves;
    opening::OpeningBoard board;
    getLine(node, moves, board);

    int v = (int)value.toDouble();
    m_opBook.updateValue(board.key(), v, m_showingSide, saveTo);
}
