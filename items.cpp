#include "items.h"
#include "texture_mgr.h"

/********************************************************************/

SDL_Texture* BasicItem::texture() {
    if( texture_ == nullptr ) {
        texture_ = TextureMgr::instance()->getItemTexture(name());
    }
    return texture_;
}

BasicItem& BasicItem::null() {
    static BasicItem none("none");
    return none;
}

/********************************************************************/

CountedItem::CountedItem() : item_(BasicItem("none")), count_(0) {
}

CountedItem::CountedItem(const BasicItem& item, int count) : item_(item), count_(count) {
}

CountedItem::~CountedItem() {
}

// Cannot contain more than 99 items in a slot
// return the number of items not added in this CountedItem
int CountedItem::addItem(int count) {
    if( count_ == CountedItem::maxCount() ) {
        return count;
    }
    if( count + count_ <= CountedItem::maxCount() ) {
        count_ += count;
        return 0;
    }
    int item_added = CountedItem::maxCount() - count_;
    count_ = CountedItem::maxCount();
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
    // invalidate the basicitem
    item_ = BasicItem("none");
    return count - item_removed;
}

bool CountedItem::isNull() const {
    return item_.name() == "none";
}
