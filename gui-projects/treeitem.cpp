
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

#include <QStringList>

#include "treeitem.h"
#include "../source/OpBoard.h"

TreeItem::TreeItem(int8_t _from, int8_t _dest, int16_t _value, int16_t _learntValue, std::string _sanMoveString, TreeItem *parent)
{
    prop = 0;
    parentItem = parent;
    from = _from;
    dest = _dest;
    value = _value;
    learntValue = _learntValue;
    sanMoveString = _sanMoveString;
}

TreeItem::~TreeItem()
{
    parentItem = nullptr;
    qDeleteAll(childItems);
}

TreeItem *TreeItem::child(int number)
{
    return childItems.value(number);
}

int TreeItem::childCount() const
{
    return childItems.isEmpty() ? ((prop & Prop_hasExpanded) ? 0 : 1) : childItems.count();
}

int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

int TreeItem::columnCount() const
{
    return 1;
}

QString TreeItem::data() const
{
    auto moveString = opening::Hist::moveString_coordinate(from, dest);

    auto str = QString(moveString.c_str());
    if (prop & Prop_hasValue) {
        str += QString("/%1").arg(value);
    }
    return str;
}

void TreeItem::addChild(TreeItem* child)
{
    child->parentItem = this;
    childItems.append(child);
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

const TreeItem *TreeItem::parent() const
{
    return parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}
