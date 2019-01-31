#include "object.h"
#include "translator.h"

#include <iostream>
#include <algorithm>
#include "sdl_camera.h"

/********************************************************************/

Object::Object(
    const std::string& icon_name,
    const std::string& user_name,
    const std::string& name
) : icon_name_(icon_name), user_name_(user_name), name_(name) {
}

SDL_Texture* Object::getTexture(SDLCamera* camera, int /*index*/) {
    if( images_.size() == 0 ) {
        SDL_Surface* surface = Utility::IMGLoad(icon_name_);
        width_ = surface->w;
        height_ = surface->h;
        SDL_Renderer* main_renderer = camera->main_renderer();
        images_.push_back( SDL_CreateTextureFromSurface(main_renderer, surface) );
        SDL_FreeSurface(surface);
    }
    return images_[0]; // TODO : take into account the orientation of the object
}

void Object::render(SDLCamera* camera, const SDL_Rect& original_rect) {
    SDL_Rect rect(original_rect);
    SDL_Texture* texture = getTexture(camera);
    rect.w = pixel_width()*camera->scale();
    rect.h = pixel_height()*camera->scale();
    rect.y = rect.y + Utility::tileSize*camera->scale() - rect.h;
    camera->displayTexture( texture, &rect);
}

/********************************************************************/

Chest::Chest(int size) : Object("objects/chest.png", tr("Chest"), "chest"), max_size_(size) {
    // debug
    addItem(BasicItem("stone"), 5);
    addItem(BasicItem("sand"), 3);
    addItem(BasicItem("copper_cable"), 2);
    addItem(BasicItem("coal"), 10);
    // end debug
}

void Chest::render(SDLCamera* sdl_camera, const SDL_Rect& rect) {
    // first: display texture of the chest
    Object::render(sdl_camera, rect);
    // second: display items stored in the chest
    const std::vector<CountedItem>& items = this->items();
    int scaled_tile_size = Utility::tileSize * sdl_camera->scale();
    for( int i = 0; i < std::min(4, int(items.size())); i++ ) {
        const CountedItem& cur = items[i];

        SDL_Texture* item_texture = cur.texture();
        SDL_Rect dest;
        int half_scaled_tile_size = scaled_tile_size/2;
        dest.x = rect.x; // + half_scaled_tile_size/2;
        if( i == 1 ) dest.x += half_scaled_tile_size;
        if( i == 3 ) dest.x += half_scaled_tile_size;
        dest.y = rect.y; // + half_scaled_tile_size/2;
        if( i == 2 ) dest.y += half_scaled_tile_size;
        if( i == 3 ) dest.y += half_scaled_tile_size;
        dest.w = half_scaled_tile_size;
        dest.h = half_scaled_tile_size;
        sdl_camera->displayTexture(item_texture, &dest);
    }

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

const std::string Chest::tooltip() const {
    std::string text = Object::tooltip();
    if( items_.size() == 0 ) {
        return text;
    }
    text.append(":");
    for(auto item : items_) {
        text.append(" ");
        std::string s = tr(item.item().name());
        std::replace(s.begin(), s.end(), '_', ' ');
        text.append(s);
        text.append("x");
        text.append( Utility::itos(item.count()) );
    }
    return text;
}

/********************************************************************/

StoneFurnace::StoneFurnace() : Object("objects/stone_furnace.png", tr("StoneFurnace"), "stone_furnace") {
}

WorkBench::WorkBench() : Object("objects/workbench.png", tr("WorkBench"), "workbench") {
}

ElectricFurnace::ElectricFurnace() : Object("objects/electric_furnace.png", tr("ElectricFurnace"), "electric_furnace") {
}

/********************************************************************/
