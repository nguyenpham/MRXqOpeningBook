#include "treeitem.h"

#include <QStringList>

#include "../source/OpBoard.h"

TreeItem::TreeItem(int8_t _from, int8_t _dest, int16_t _value, const std::string& _sanMoveString, TreeItem *parent)
{
    prop = 0;
    parentItem = parent;
    from = _from;
    dest = _dest;
    value = _value;
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

//bool TreeItem::insertChildren(int position, int count, int columns)
//{
//    if (position < 0 || position > childItems.size())
//        return false;

//    for (int row = 0; row < count; ++row) {
//        QVector<QVariant> data(columns);
//        TreeItem *item = new TreeItem(data, this);
//        childItems.insert(position, item);
//    }

//    return true;
//}

//bool TreeItem::insertColumns(int position, int columns)
//{
//    if (position < 0 || position > itemData.size())
//        return false;

//    for (int column = 0; column < columns; ++column)
//        itemData.insert(position, QVariant());

//    foreach (TreeItem *child, childItems)
//        child->insertColumns(position, columns);

//    return true;
//}

TreeItem *TreeItem::parent()
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

//bool TreeItem::removeColumns(int position, int columns)
//{
//    if (position < 0 || position + columns > itemData.size())
//        return false;

//    for (int column = 0; column < columns; ++column)
//        itemData.remove(position);

//    foreach (TreeItem *child, childItems)
//        child->removeColumns(position, columns);

//    return true;
//}

//bool TreeItem::setData(int column, const QVariant &value)
//{
//    if (column < 0 || column >= itemData.size())
//        return false;

//    itemData[column] = value;
//    return true;
//}
