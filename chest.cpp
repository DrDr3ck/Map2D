#include "chest.h"

#include <iostream>
#include "sdl_camera.h"

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

Object::Object(const std::string& icon_name) : icon_name_(icon_name) {
}

SDL_Texture* Object::getTexture(SDLCamera* camera, int index) {
    if( images_.size() == 0 ) {
        SDL_Surface* surface = Utility::IMGLoad(icon_name_);
        width_ = surface->w;
        height_ = surface->h;
        SDL_Renderer* main_renderer = camera->main_renderer();
        images_.push_back( SDL_CreateTextureFromSurface(main_renderer, surface) );
        SDL_FreeSurface(surface);
    }
    return images_[0]; // TODO
}

void Object::render(SDLCamera* camera, const SDL_Rect& original_rect) {
    SDL_Rect rect(original_rect);
    SDL_Texture* texture = getTexture(camera);
    rect.w = pixel_width();
    rect.h = pixel_height();
    rect.y = rect.y + 64*camera->scale() - rect.h;
    camera->displayTexture( texture, &rect);
}

/********************************************************************/

Chest::Chest(int size) : Object("chest.png"), max_size_(size) {
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