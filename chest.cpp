#include "chest.h"

#include <iostream>

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

/********************************************************************/

Chest::Chest(int size) : max_size_(size) {
}

// return the number of items not added in this Chest
int Chest::addItem(const BasicItem& item, int count) {
    int total_count = count;
    // find item if already in the chest
    for( auto& counted_item : items_ ) {
        if( counted_item.item() == item ) {
            total_count = counted_item.addItem(total_count);
            if( total_count == 0 ) {
                return 0;
            }
        }
    }
    // there are still some items to add...
    while( max_size_ >= int(items_.size()) ) {
        int c = total_count;
        if( total_count > 99 ) {
            c = 99;
        }
        items_.push_back(CountedItem(item, c));
        total_count -= c;
        if( total_count == 0 ) {
            return 0;
        }
    }
    return total_count;
}

// return the number of items that cannot be removed from the Chest
int Chest::removeItem(const BasicItem& item, int count) {
    // find item in the chest if any
    int total_count = count;
    // find item if already in the chest
    for( auto& counted_item : items_ ) {
        if( counted_item.item() == item ) {
            total_count = counted_item.removeItem(total_count);
            if( total_count == 0 ) {
                break;
            }
        }
    }
    // remove all items that are empty
    bool to_remove = true;
    while( to_remove ) {
        int index = 0;
        for( auto counted_item : items_ ) {
            if( counted_item.count() == 0 ) {
                break;
            }
            index++;
        }
        if( index < int(items_.size()) && items_[index].count() == 0 ) {
            items_.erase(items_.begin()+index);
        } else {
            to_remove = false;
        }
    }
    return total_count;
}

/********************************************************************/
