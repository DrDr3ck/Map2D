#include "object.h"
#include "translator.h"

#include <iostream>
#include <algorithm>
#include "sdl_camera.h"
#include "craft_mgr.h"

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
        // Transparency with green color
        Uint32 key = SDL_MapRGB(surface->format, 0, 255, 0);
        SDL_SetColorKey(surface , SDL_TRUE, key);
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
    // object toolbar when crafting
    if( percentageAccomplished() != -1 ) {
        int activity_percent = percentageAccomplished();
        static int offset = 3;
        static int toolbar_height = 9;
        SDL_Rect activity_rect = {rect.x+offset,rect.y+rect.h-offset-toolbar_height,0,0};
        SDL_SetRenderDrawColor( camera->main_renderer(), 0, 0, 250, 255 );
        int width = activity_percent / 100.0 * (Utility::tileSize*camera->scale()-2*offset);
        activity_rect.w = width;
        activity_rect.h = toolbar_height;
        SDL_RenderFillRect( camera->main_renderer(), &activity_rect );
        SDL_SetRenderDrawColor( camera->main_renderer(), 0, 128, 255, 255 );
        activity_rect.w = Utility::tileSize*camera->scale()-2*offset;
        activity_rect.h = toolbar_height;
        SDL_RenderDrawRect( camera->main_renderer(), &activity_rect );
    }
}

int Object::getNodeCount() const {
    return crafts_.size();
}

// <craft nb="10">glass</craft>
const std::string Object::getNodeName(int node_index) const {
    std::ignore = node_index;
    return "craft";
}

int Object::getAttributeCount(int node_index) const {
    std::ignore = node_index;
    return 1; // nb
}

const std::string Object::getAttributeName(int node_index,int attr_index) const {
    std::ignore = node_index;
    std::ignore = attr_index;
    return "nb";
}

const std::string Object::getAttributeValue(int node_index,int attr_index) const {
    std::ignore = attr_index;
    int occ = crafts_[node_index].second;
    return Utility::itos(occ);
}

const std::string Object::getNodeValue(int node_index) const {
    Craft* craft = crafts_[node_index].first;
    return craft->name();
}

const std::string Object::getNodeString(int node_index) const {
    std::ignore = node_index;
    return "";
}

void Object::setNode(const std::string& node_name, std::vector<std::pair<std::string, std::string>> attributes, const std::string& value) {
    if( node_name == "craft" ) {
        if( attributes.size() != 1 ) return;
        if( attributes[0].first != "nb" ) return;

        std::string nb_str = attributes[0].second;
        Craft* craft = CraftMgr::instance()->findCraft(value, this->name());
        addCraft(craft,atoi( nb_str.c_str() ));
    }
}

void Object::addCraft(Craft* craft, int occ) {
    crafts_.push_back(std::pair<Craft*,int>(craft, occ));
}

void Object::animate(double delta_ms) { // TODO
    if( inPause() ) return;
    if( crafts_.size() == 0 ) return;

    if( has_ingredients_ && cur_craft_ != nullptr && cur_craft_time_ms_ > delta_ms ) {
        cur_craft_time_ms_ -= delta_ms;
    }

    if( cur_craft_ == nullptr ) {
        has_ingredients_ = false;
        cur_craft_ = crafts_.at(0).first;
    }

    if( !hasIngredients() ) {
        checkIngredients();
    }

    if( hasIngredients() ) {
        if( cur_craft_time_ms_ <= delta_ms ) {
            cur_craft_time_ms_ = 0;
            int occ = crafts_.at(0).second;
            MapView* map_view = MapView::cur_map;
            map_view->data()->store(BasicItem(cur_craft_->name()), tilePosition()); // TODO keep item in the furnace if store returns false ?
            if( occ > 1) {
                crafts_.at(0).second = occ - 1;
            } else {
                crafts_.erase(crafts_.begin());
            }
            cur_craft_ = nullptr;
        }
    }
}

bool Object::hasIngredients() const {
    return has_ingredients_;
}

void Object::checkIngredients() { // TODO
    if( cur_craft_ == nullptr ) return;
    // get list of ingredients and check that ingredients are available in a chest
    const std::vector<CountedItem>& items = cur_craft_->getItems();
    // if ingredients are all present, start craft
    for( auto counted_item : items ) {

    }
    // remove all ingredients from various chests
    has_ingredients_ = true;
    cur_craft_time_ms_ = cur_craft_->time()*1000;
    max_craft_time_ms_ = cur_craft_time_ms_;
    // otherwise: do not start the craft !!
}

/*!
 * if an item is currently crafted, returns the percentage of accomplishement between 0 and 100
 * if no craft, returns -1
 */
int Object::percentageAccomplished() const {
    if( cur_craft_ == nullptr ) {
        return -1;
    }
    if( max_craft_time_ms_ == 0 ) {
        return -1;
    }
    return (cur_craft_time_ms_*100/max_craft_time_ms_);
}

/********************************************************************/

Chest::Chest(
    const std::string& icon_name,
    const std::string& user_name,
    const std::string& name,
    int size
) : Object(icon_name, user_name, name), max_size_(size) {
    is_crafter_ = false;
}

Chest::Chest(int size) : Object("objects/chest.png", tr("Chest"), "chest"), max_size_(size) {
    is_crafter_ = false;
}

void Chest::setNode(const std::string& node_name, std::vector<Attr> attributes, const std::string& value) {
    if( node_name == "counted_item" ) {
        if( attributes.size() != 1 ) return;
        if( attributes[0].first != "nb" ) return;

        std::string nb_str = attributes[0].second;
        addItem(BasicItem(value), atoi( nb_str.c_str() ));
    }
}

int Chest::getNodeCount() const {
    return items_.size();
}

const std::string Chest::getNodeName(int node_index) const {
    std::ignore = node_index;
    return "counted_item";
}

int Chest::getAttributeCount(int node_index) const {
    std::ignore = node_index;
    return 1;
}

const std::string Chest::getAttributeName(int /*node_index*/,int /*attr_index*/) const {
    return "nb";
}

const std::string Chest::getAttributeValue(int node_index,int /*attr_index*/) const {
    return Utility::itos(items_[node_index].count());
}

const std::string Chest::getNodeValue(int node_index) const {
    return items_[node_index].item().name();
}

const std::string Chest::getNodeString(int node_index) const {
    std::string result = items_[node_index].item().name();
    result = tr(result) + " x " + getAttributeValue(node_index,0);
    return result;
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
            CommandCenter::addItems(item, count-total_count);
            if( total_count == 0 ) {
                return 0;
            }
        }
    }
    // there are still some items to add...
    while( max_size_ > int(items_.size()) ) {
        int c = total_count;
        if( total_count > CountedItem::maxCount() ) {
            c = CountedItem::maxCount();
        }
        items_.push_back(CountedItem(item, c));
        CommandCenter::addItems(item, c);
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
            CommandCenter::removeItems(item, count-total_count);
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
        text.append("(");
        text.append( Utility::itos(item.count()) );
        text.append(")");
    }
    return text;
}

/********************************************************************/

IronChest::IronChest(int size) : Chest("objects/iron_chest.png", tr("IronChest"), "iron_chest", size) {
}

/********************************************************************/

Furnace::Furnace(
     const std::string& icon_name,
     const std::string& user_name,
     const std::string& name
) : Object(icon_name, user_name, name) {
}

void Furnace::animate(double delta_ms) {
    if( inPause() ) return;
    if( crafts_.size() == 0 ) return;

    // no fuel in furnace or no storage : get fuel
    if( fuel_time_ms_ == 0 || fuel_.count() == 0 ) {
        getFuel();
    }
    if( fuel_time_ms_ > 0 ) {
        Object::animate(delta_ms);
    }

    // do not use coal if craft is not ready
    if( !has_ingredients_ ) return;
    if( fuel_time_ms_ > delta_ms ) {
        fuel_time_ms_ -= delta_ms;
    } else {
        fuel_time_ms_ = 0;
    }
}

const std::string Furnace::tooltip() const {
    std::string text = Object::tooltip();
    if( fuel_time_ms_ > 0 ) {
        text.append("\n");
        text.append( tr("fuel time (in sec): ") ) ;
        text.append( Utility::itos(int(fuel_time_ms_/1000)) );
    }
    return text;
}

void Furnace::getFuel() {
    if( fuel_time_ms_ == 0 && !fuel_.isNull() ) {
        fuel_time_ms_ = 80000;
        fuel_.removeItem(1);
    }
    // find fuel in the nearest chest
    // or ask a robot to get fuel : TODO
    MapView* map_view = MapView::cur_map;
    Chest* chest = static_cast<Chest*>(map_view->data()->getAssociatedChest(tilePosition()));
    if( chest == nullptr ) { return; }
    // find coal in chest
    if( chest->removeItem(BasicItem("coal"), 1) == 0 ) {
        // coal has been found and removed from the chest,
        // furnace can use it
        if( fuel_time_ms_ == 0 ) {
            fuel_time_ms_ = 80000;
        } else {
            if( fuel_.isNull() ) {
                fuel_ = CountedItem(BasicItem("coal"));
            } else {
                fuel_.addItem(1);
            }
        }
    }
}

// save also fuel and fuel_time_ms
int Furnace::getNodeCount() const {
    return Object::getNodeCount() + 1;
}

// <craft nb="10">glass</craft>
const std::string Furnace::getNodeName(int node_index) const {
    if( node_index == Object::getNodeCount() ) {
        return "fuel";
    }
    return "craft";
}

int Furnace::getAttributeCount(int node_index) const {
    if( node_index == Object::getNodeCount() ) {
        return 1; // time in ms
    }
    return 1; // nb
}

const std::string Furnace::getAttributeName(int node_index,int attr_index) const {
    if( node_index == Object::getNodeCount() ) {
        return "fuel_time_ms";
    }
    std::ignore = attr_index;
    return "nb";
}

const std::string Furnace::getAttributeValue(int node_index,int attr_index) const {
    std::ignore = attr_index;
    if( node_index == Object::getNodeCount() ) {
        return Utility::itos(fuel_time_ms_);
    }
    int occ = crafts_[node_index].second;
    return Utility::itos(occ);
}

const std::string Furnace::getNodeValue(int node_index) const {
    if( node_index == Object::getNodeCount() ) {
        return Utility::itos( fuel_.count() ); // nb fuel item
    }
    Craft* craft = crafts_[node_index].first;
    return craft->name();
}

void Furnace::setNode(const std::string& node_name, std::vector<std::pair<std::string, std::string>> attributes, const std::string& value) {
    if( node_name == "craft" ) {
        if( attributes.size() != 1 ) return;
        if( attributes[0].first != "nb" ) return;

        std::string nb_str = attributes[0].second;
        Craft* craft = CraftMgr::instance()->findCraft(value, this->name());
        addCraft(craft,atoi( nb_str.c_str() ));
    } else if( node_name == "fuel" ) {
        if( attributes.size() != 1 ) return;
        if( attributes[0].first != "fuel_time_ms" ) return;

        std::string nb_str = attributes[0].second;
        // value is the number of fuel
        // attr is the fuel time in ms
        int nb_fuel = atoi(value.c_str());
        if( nb_fuel == 0 ) {
            fuel_ = CountedItem();
        } else {
            fuel_ = CountedItem(BasicItem("coal"), nb_fuel);
        }
        fuel_time_ms_ = atoi(nb_str.c_str());
    }
}

StoneFurnace::StoneFurnace() : Furnace("objects/stone_furnace.png", tr("StoneFurnace"), "stone_furnace") {
}

Breaker::Breaker() : Object("objects/breaker.png", tr("Breaker"), "breaker") {
}

WorkBench::WorkBench() : Object("objects/workbench.png", tr("WorkBench"), "workbench") {
}

Assembler::Assembler() : Object("objects/assembler.png", tr("Assembler"), "assembler") {
}

ElectricFurnace::ElectricFurnace() : Furnace("objects/electric_furnace.png", tr("ElectricFurnace"), "electric_furnace") {
}

/********************************************************************/

CommandCenter* CommandCenter::cur_command_center = nullptr;

CommandCenter::CommandCenter() : Object("objects/command_center.png", tr("CommandCenter"), "command_center") {
    if( cur_command_center == nullptr ) {
        cur_command_center = this;
    }
}

const std::vector<CountedItem>& CommandCenter::storedItems() const {
    return stored_items_;
}

std::vector<CountedItem>& CommandCenter::storedItems() {
    return stored_items_;
}

void CommandCenter::addItems(const BasicItem& item, int nb) {
    CommandCenter* cc = CommandCenter::cur_command_center;
    if( cc == nullptr ) return;
    std::vector<CountedItem>& stored_items = cc->storedItems();
    for( auto& counted_item : stored_items ) {
        if( counted_item.item().name() == item.name() ) {
            counted_item.addItem(nb);
            return;
        }
    }
    stored_items.push_back( CountedItem(item, nb) );
}

int CommandCenter::countedItems(const BasicItem& item) const {
    for( auto& counted_item : storedItems() ) {
        if( counted_item.item().name() == item.name() ) {
            return counted_item.count();
        }
    }
    return 0;
}

/*!
 * \return the number of items not removed. if 0, all items have been removed
 */
int CommandCenter::removeItems(const BasicItem& item, int nb) {
    int total = nb;
    CommandCenter* cc = CommandCenter::cur_command_center;
    if( cc == nullptr ) return total;
    std::vector<CountedItem>& stored_items = cc->storedItems();
    for( auto& counted_item : stored_items ) {
        if( counted_item.item().name() == item.name() ) {
            if( nb <= counted_item.count() ) {
                counted_item.removeItem(nb);
                total = 0;
            } else {
                total = total - counted_item.count();
                counted_item.removeItem(counted_item.count());
            }
        }
    }
    return total;
}

void CommandCenter::init(CommandCenter* cc, std::vector<Chest*> chests) {
    if( cc == nullptr ) { return; }
    for( auto chest : chests ) {
        const std::vector<CountedItem>& counted_items = chest->items();
        for( auto counted_item : counted_items ) {
            cc->addItems(counted_item.item(), counted_item.count());
        }
    }
}

void CommandCenter::reset() {
    stored_items_.clear();
}

/********************************************************************/
