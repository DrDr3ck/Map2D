#include "items.h"

/********************************************************************/

CountedItem::CountedItem(const BasicItem& item, int count) : item_(item), count_(count) {
}

CountedItem::~CountedItem() {
}

// Cannot contain more than 99 items in a slot
// return the number of items not added in this CountedItem
int CountedItem::addItem(int count) {
    if( count_ == 99 ) {
        return count;
    }
    if( count + count_ <= 99 ) {
        count_ += count;
        return 0;
    }
    int item_added = 99 - count_;
    count_ = 99;
    return count - item_added;
}

// return the number of items that cannot be removed from the CountedItem
int CountedItem::removeItem(int count) {
    if( count_ == 0 ) {
        return count;
    }
    if( count <= count_) {
        count_ -= count;
        return 0;
    }
    int item_removed = count_;
    count_ = 0;
    return count - item_removed;
}
