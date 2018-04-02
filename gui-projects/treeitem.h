
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

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

class TreeItem
{
public:
    explicit TreeItem(int8_t from = 0, int8_t dest = 0, int16_t value = 0, int16_t learntValue = -1, std::string sanMoveString = "", TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QString data() const;
    void addChild(TreeItem* child);
    TreeItem *parent();
    const TreeItem *parent() const;
    bool removeChildren(int position, int count);
    int childNumber() const;

    int getIndexRow() const {
        if (parentItem) {
            for(int i = 0; i < parentItem->childItems.size(); i++) {
                if (parentItem->childItems.at(i) == this) {
                    return i;
                }
            }
        }
        return 0;
    }

public:
    static const int Prop_hasExpanded   = (1 << 0);
    static const int Prop_hasValue      = (1 << 1);

    int8_t prop;
    int8_t from, dest;
    int16_t value, learntValue;

    QList<TreeItem*> childItems;

private:
    TreeItem *parentItem;

    std::string sanMoveString;
};

#endif // TREEITEM_H
