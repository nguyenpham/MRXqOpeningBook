
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

    enum TreeModelRoles {
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

    Q_INVOKABLE void updateValue(const QModelIndex &index, const QVariant &value, int saveTo);
    Q_INVOKABLE QVariantList selectionChanged(const QModelIndex &index);
    Q_INVOKABLE bool loadOpening(const QString& openingPath, int sd);
    Q_INVOKABLE bool expand(const QModelIndex &index);
    Q_INVOKABLE bool removeExpandedItems(const QModelIndex &index);

private:
    bool _expand(TreeItem* node);
    bool buildTree(TreeItem* treeItem, opening::OpeningBoard& board, int sd, int ply);
    void getLine(const TreeItem* node, std::vector<opening::MoveCore>& moves, opening::OpeningBoard& board);
    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;
    QSet<TreeItem*> m_workingItems;

    opening::OpBook m_opBook;
    opening::Side m_showingSide;
};

#endif // OPENINGTREEMODEL_H
