#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

class TreeItem
{
public:
    explicit TreeItem(int8_t from, int8_t dest, int16_t value, const std::string& sanMoveString, TreeItem *parent = 0);
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
    int16_t value;

    QList<TreeItem*> childItems;

private:
    TreeItem *parentItem;

    std::string sanMoveString;
};

#endif // TREEITEM_H
