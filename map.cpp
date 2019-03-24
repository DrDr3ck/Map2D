#include "map.h"

#include <cmath>

#include "SDL2/SDL_image.h"
#include "character.h"
#include "perlin_noise.h"
#include "sdl_background.h"
#include "logger.h"
#include "translator.h"
#include <iostream>
#include <fstream>
#include <algorithm>

/********************************************************************/

TerrainType* MapUtility::readColorMap(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        Logger::error() << "unable to open file for load: " << filename << Logger::endl;
        return nullptr;
    }
    int size = 6;
    TerrainType* terrain = new TerrainType[size];
    // TODO read colormap
    return terrain;
}

bool MapUtility::saveColorMap(const std::string& filename, TerrainType* regions, int regionCount) {
    std::ofstream file(filename);
    if (!file) {
        Logger::error() << "Unable to open file for colormap: " << filename << Logger::endl;
        return false;
    }
    file << "{" << std::endl;
    for( int region = 0; region < regionCount; region++) {
        file << "    \"" << regions[region].label.data() << "\" : {" << std::endl;
        int r = regions[region].red;
        int g = regions[region].green;
        int b = regions[region].blue;
        int h = regions[region].height;
        file << "        \"height\": " << h << "," << std::endl;
        file << "        \"red\": " << r << "," << std::endl;
        file << "        \"green\": " << g << "," << std::endl;
        file << "        \"blue\": " << b << "," << std::endl;
        file << "    }," << std::endl;
    }
    file << "}" << std::endl;

    file.close();
    return true;
}

/********************************************************************/

Tile::Tile(
    int id, Type cell_type, BType background_type, FType floor_type
) : id_(id), cell_type_(cell_type), occurrences_(0),
    background_type_(background_type), floor_type_(floor_type)
{
}

Tile::~Tile() {
}

int Tile::id() const {
    return id_;
}

bool Tile::isWall(const Tile& tile) {
    return tile.cell_type() == Tile::WALL;
}
bool Tile::isFloor(const Tile& tile) {
    return tile.cell_type() == Tile::FLOOR;
}
bool Tile::isDoor(const Tile& tile) {
    return tile.cell_type() == Tile::DOOR;
}

Tile::Type Tile::cell_type() const {
    return cell_type_;
}

// occurrences de la tuile : par exemple, 10 cailloux
int Tile::occurrences() const {
    return occurrences_;
}

Tile::BType Tile::background_type() const {
    return background_type_;
}

Tile::FType Tile::floor_type() const {
    return floor_type_;
}

void Tile::setOccurrences(int value) {
    occurrences_ = value;
}

void Tile::setTile(int id, Type cell_type, BType background_type, FType floor_type) {
    id_ = id;
    cell_type_ = cell_type;
    background_type_ = background_type;
    floor_type_ = floor_type;
}

void Tile::setCellTile(int id, Type cell_type) {
    id_ = id;
    cell_type_ = cell_type;
}

void Tile::setBackgroundTile(int id, BType background_type) {
    id_ = id;
    background_type_ = background_type;
}

void Tile::setFloorTile(int id, FType floor_type) {
    id_ = id;
    floor_type_ = floor_type;
}

std::string Tile::typeTileToString(Tile::Type type) {
    if( type == Tile::BLOCK ) return tr("Block");
    if( type == Tile::DOOR ) return tr("Door");
    if( type == Tile::FLOOR ) return tr("Floor");
    if( type == Tile::WALL ) return tr("Wall");
    if( type == Tile::EMPTY ) return tr("Nothing...");
    Logger::error() << "unable to find string for type: " << type << Logger::endl;
    return "NONE";
}

std::string Tile::btypeTileToString(Tile::BType type) {
    if( type == Tile::NONE ) return tr("None");
    if( type == Tile::WATER ) return tr("Water");
    if( type == Tile::SAND ) return tr("Sand");
    if( type == Tile::COPPER ) return tr("Copper");
    if( type == Tile::IRON ) return tr("Iron");
    if( type == Tile::DIRT ) return tr("Dirt");
    if( type == Tile::GRASS ) return tr("Grass");
    if( type == Tile::COAL ) return tr("Coal");
    if( type == Tile::ROCK ) return tr("Rock");
    Logger::error() << "unable to find string for background type: " << type << Logger::endl;
    return "NONE";
}

void Tile::addItem(const BasicItem& item, int nb) {
    for( auto& counted_item : counted_items_ ) {
        if( item.name() == counted_item.item().name() ) {
            counted_item.addItem(nb);
            return;
        }
    }
    counted_items_.push_back(CountedItem(item, nb));
}

std::string Tile::removeItem(const BasicItem& item, int nb) {
    if( counted_items_.size() == 0 ) {
        return "none";
    }
    int index = 0;
    for( auto& counted_item : counted_items_ ) {
        if( item.name() == counted_item.item().name() ) {
            counted_item.removeItem(nb);
            std::string item_name = counted_item.item().name();
            if( counted_item.count() == 0 ) {
                counted_items_.erase(counted_items_.begin() + index);
            }
            return item_name;
        }
        index++;
    }
    return "none";
}

/********************************************************************/

MapData::MapData(int width, int height) : width_(width), height_(height) {
    tiles_ = new Tile[width*height];
    map_image_name_ = "out.png";
}

MapData::~MapData() {
    delete[] tiles_;
}

void MapData::reset(int width, int height) {
    delete[] tiles_;
    width_ = width;
    height_ = height;
    tiles_ = new Tile[width*height];
}

void MapData::setMapImageName(const std::string& map_image_name) {
    map_image_name_ = map_image_name;
}

const std::string& MapData::getMapImageName() const {
    return map_image_name_;
}

void MapData::addWall(int x, int y) {
    // take a look to the tile around the position
    // Tile(int id=0, Type type=EMPTY);
    if( Tile::isWall(tile(x,y)) ) {
        // already a wall
        return;
    }
    int id = 0;
    if( x > 0 ) {
        if( Tile::isWall(tile(x-1,y)) ) {
            id += 1;
            Tile& tile_x_1 = tile(x-1,y);
            tile_x_1.setCellTile( tile_x_1.id() + 4, Tile::WALL);
        }
    }
    if( x < width_-1 ) {
        if( Tile::isWall(tile(x+1,y)) ) {
            id += 4;
            Tile& tile_x_1 = tile(x+1,y);
            tile_x_1.setCellTile( tile_x_1.id() + 1, Tile::WALL);
        }
    }
    if( y < height_-1 ) {
        if( Tile::isWall(tile(x,y+1)) ) {
            id += 2;
            Tile& tile_y_1 = tile(x,y+1);
            tile_y_1.setCellTile( tile_y_1.id() + 8, Tile::WALL);
        }
    }
    if( y > 0 ) {
        if( Tile::isWall(tile(x,y-1)) ) {
            id += 8;
            Tile& tile_y_1 = tile(x,y-1);
            tile_y_1.setCellTile( tile_y_1.id() + 2, Tile::WALL);
        }
    }
    tile(x,y).setCellTile(id, Tile::WALL);
}

void MapData::removeWall(int x, int y) {
    if( !Tile::isWall(tile(x,y)) ) {
        // not a wall
        return;
    }
    tile(x,y).setCellTile(0, Tile::FLOOR);
    if( x > 0 ) {
        if( Tile::isWall(tile(x-1,y)) ) {
            Tile& tile_x_1 = tile(x-1,y);
            tile_x_1.setCellTile( tile_x_1.id() - 4, Tile::WALL);
        }
    }
    if( x < width_-1 ) {
        if( Tile::isWall(tile(x+1,y)) ) {
            Tile& tile_x_1 = tile(x+1,y);
            tile_x_1.setCellTile( tile_x_1.id() - 1, Tile::WALL);
        }
    }
    if( y < height_-1 ) {
        if( Tile::isWall(tile(x,y+1)) ) {
            Tile& tile_y_1 = tile(x,y+1);
            tile_y_1.setCellTile( tile_y_1.id() - 8, Tile::WALL);
        }
    }
    if( y > 0 ) {
        if( Tile::isWall(tile(x,y-1)) ) {
            Tile& tile_y_1 = tile(x,y-1);
            tile_y_1.setCellTile( tile_y_1.id() - 2, Tile::WALL);
        }
    }
}

void MapData::addFloor(int x, int y) {
    if( Tile::isFloor(tile(x,y)) || Tile::isWall(tile(x,y)) ) { // cannot build floor on a wall
        // already a floor
        return;
    }
    Tile& cur = tile(x,y);
    cur.setCellTile(cur.id(), Tile::FLOOR);
}

void MapData::removeFloor(int x, int y) {
    if( !Tile::isFloor(tile(x,y)) ) {
        // already empty
        return;
    }
    Tile& cur = tile(x,y);
    cur.setCellTile(cur.id(), Tile::EMPTY);
}

void MapData::addDoor(int x, int y) {
    if( Tile::isDoor(tile(x,y)) || Tile::isWall(tile(x,y)) ) { // cannot build door on a wall or another door
        // already a door
        return;
    }
    int id = 0;
    Tile& cur = tile(x,y);
    if( y > 0 ) {
        if( Tile::isWall(tile(x,y-1)) ) {
            id=1;
        }
    }
    if( y < height_-1 ) {
        if( Tile::isWall(tile(x,y+1)) ) {
            id=1;
        }
    }
    cur.setCellTile(id, Tile::DOOR);
}

void MapData::removeDoor(int x, int y) {
    if( !Tile::isDoor(tile(x,y)) ) {
        // already empty
        return;
    }
    Tile& cur = tile(x,y);
    cur.setCellTile(cur.id(), Tile::FLOOR);
}

void MapData::cleanItemFromTile(int x,int y,Character* people) {
    Tile& cur = tile(x,y);
    if( cur.counted_items().size() == 0 ) {
        // nothing to do, no counted item on this tile
        return;
    }
    CountedItem& counted_item = cur.counted_items().at(0);
    int max_carry = people->maxCarriable(counted_item.item()); // get the maximum of item the robot can carry
    int max_items = counted_item.count();
    if( max_items >= max_carry ) {
        people->carryItem(counted_item.item(), max_carry);
        counted_item.removeItem(max_carry);
    } else {
        people->carryItem(counted_item.item(), max_items);
        counted_item.removeItem(max_items);
    }
}

void MapData::extractItemFromTile(int x,int y) {
    Tile& cur = tile(x,y);
    if( cur.background_type() == Tile::ROCK ) {
        // extraction of a rock
        if( cur.occurrences() > 0 ) {
            cur.setOccurrences( cur.occurrences()-1 );
            // create an item stone and put it on the 'cur' Tile
            cur.addItem(BasicItem("stone"), 1);

            // TODO should we change rock into dirt if occurrences == 0 ?
        }
    } else if( cur.background_type() == Tile::COPPER ) {
        // extraction of copper
        if( cur.occurrences() > 0 ) {
            cur.setOccurrences( cur.occurrences()-1 );
            // create an item stone and put it on the 'cur' Tile
            cur.addItem(BasicItem("copper_ore"), 1);

            // TODO should we change sand into dirt if occurrences == 0 ?
        }
    } else if( cur.background_type() == Tile::IRON ) {
        // extraction of iron
        if( cur.occurrences() > 0 ) {
            cur.setOccurrences( cur.occurrences()-1 );
            // create an item stone and put it on the 'cur' Tile
            cur.addItem(BasicItem("iron_ore"), 1);

            // TODO should we change sand into dirt if occurrences == 0 ?
        }
    } else if( cur.background_type() == Tile::SAND ) {
        // extraction of a sand
        if( cur.occurrences() > 0 ) {
            cur.setOccurrences( cur.occurrences()-1 );
            // create an item stone and put it on the 'cur' Tile
            cur.addItem(BasicItem("sand"), 1);

            // TODO should we change sand into dirt if occurrences == 0 ?
        }
    } else if( cur.background_type() == Tile::COAL ) {
        // extraction of coal
        if( cur.occurrences() > 0 ) {
            cur.setOccurrences( cur.occurrences()-1 );
            // create an item stone and put it on the 'cur' Tile
            cur.addItem(BasicItem("coal"), 1);

            // TODO should we change sand into dirt if occurrences == 0 ?
        }
    }
}

bool MapData::transferItems(Character* people) {
    Tile& tile = this->tile(people->tilePosition().x, people->tilePosition().y);
    if( tile.counted_items().size() == 0 ) {
        return false; // tile has no items
    }
    if( people->maxCarriable() == 0 ) {
        return false; // robot is full
    }
    bool can_still_carry = true;
    while( can_still_carry ) {
        CountedItem counted_item = tile.counted_items().at(0);
        int max_item = std::min(people->maxCarriable(), counted_item.count());
        people->carryItem( tile.removeItem(counted_item.item(), max_item), max_item );
        if( people->maxCarriable() == 0 ) {
            can_still_carry = false; // cannot carry more
        }
        if( tile.counted_items().size() == 0 ) {
            can_still_carry = false; // no more items on the tile
        }
    }
    return true;
}

void MapData::transferItems(Character* people, Chest* chest) {
    std::vector<BasicItem> items;
    while(people->carriedItems().size() != 0 ) {
        BasicItem item = people->dropItem();
        int nb = chest->addItem( item );
        if( nb != 0 ) { // cannot transfer this item in the chest, need to keep it
            items.push_back(item);
        }
    }
    for( auto item : items ) {
        people->carryItem(item);
    }
}

Object* MapData::getAssociatedChest(Position position) {
    // find a chest at distance == 1 !!
    Object* object = getNearestChest(position);
    if( object == nullptr ) return nullptr;
    Position chest_position = object->tilePosition();
    float dist = Utility::distance(chest_position, position);
    if( dist < 2.0f ) {
        return object;
    }
    return nullptr;
}

void MapData::getEmptyGrassTilePosition(int& tile_x, int& tile_y) {
    tile_x = std::rand() % width();
    tile_y = std::rand() % height();
    Tile tile = this->tile(tile_x,tile_y);
    Position position = {tile_x, tile_y};
    Object* object = getObject(position);
    while( object != nullptr || tile.background_type() != Tile::GRASS) { // find a tile without object
        tile_x = std::rand() % width();
        tile_y = std::rand() % height();
        tile = this->tile(tile_x,tile_y);
        position = {tile_x, tile_y};
        object = getObject(position);
    }
}

Object* MapData::getNearestChest(Position position) {
    Object* nearest_chest = nullptr;
    float distance = 10000.f;
    for( Object* object : objects() ) {
        if( !Utility::endsWith(object->name(), "chest") ) continue;
        Position chest_position = object->tilePosition();
        float dist = Utility::distance(chest_position, position);
        if( dist < distance ) {
            distance = dist;
            nearest_chest = object;
        }
    }
    return nearest_chest;
}

bool MapData::removeItemFromChest(Position position, const BasicItem& item) {
    if( item.name() == "workbench" ) return true;
    if( item.name() == "chest" ) return true;
    Chest* nearest_chest_with_item = nullptr;
    float distance = 10000.f;
    for( Object* object : objects() ) {
        Chest* chest = dynamic_cast<Chest*>(object);
        if( chest == nullptr ) continue;
        const std::vector<CountedItem>& items = chest->items();
        for( auto cur_item : items ) {
            if( cur_item.item().name() == item.name() ) {
                Position chest_position = object->tilePosition();
                float dist = Utility::distance(chest_position, position);
                if( dist < distance ) {
                    distance = dist;
                    nearest_chest_with_item = chest;
                }
                break;
            }
        }
    }
    if( nearest_chest_with_item == nullptr ) return false;
    nearest_chest_with_item->removeItem(item);
    return true;
}

/*!
 * \return a Chest that can store item
 */
Object* MapData::getNearestEmptyChest(Position position, const BasicItem& item) {
    Object* nearest_chest = nullptr;
    float distance = 10000.f;
    for( Object* object : objects() ) {
        if( !Utility::endsWith(object->name(), "chest") ) continue;
        Chest* chest = static_cast<Chest*>(object);
        // check if chest can store item
        bool full = true;
        if( chest->sizeAvailable() > 0 ) {
            full = false;
        } else {
            const std::vector<CountedItem>& items = chest->items();
            for( auto cur_item : items ) {
                if( cur_item.item().name() == item.name() ) {
                    if( cur_item.count() < CountedItem::maxCount() ) {
                        full = false;
                        break;
                    }
                }
            }
        }
        if( full ) continue;
        Position chest_position = object->tilePosition();
        float dist = Utility::distance(chest_position, position);
        if( dist < distance ) {
            distance = dist;
            nearest_chest = object;
        }
    }
    return nearest_chest;
}

void MapData::addGround(int x, int y) {
    if( tile(x,y).background_type() != Tile::NONE ) {
        // already a floor
        return;
    }
    Tile& cur = tile(x,y);
    cur.setBackgroundTile(cur.id(), Tile::GRASS);
}

void MapData::removeGround(int x, int y) {
    if( tile(x,y).background_type() == Tile::NONE ) {
        // already empty
        return;
    }
    Tile& cur = tile(x,y);
    cur.setBackgroundTile(cur.id(), Tile::NONE);
}

namespace {
    bool compare_object_position(Object* o1, Object* o2) {
        Position p1 = o1->tilePosition();
        Position p2 = o2->tilePosition();
        if( p1.x == p2.x ) {
            return p1.y < p2.y;
        }
        return p1.x < p2.x;
    }
}

void MapData::addObject(Object* object, int tile_x, int tile_y) {
    Position cur_position = {tile_x,tile_y};
    object->setTilePosition(cur_position);
    objects_.push_back( object );
    // sort objects according to their position
    std::sort(objects_.begin(), objects_.end(), compare_object_position);
}

Object* MapData::getObject(Position position) {
    for( auto object : objects_ ) {
        Position cur_position = object->tilePosition();
        if( cur_position.x == position.x && cur_position.y == position.y ) {
            return object;
        }
    }
    return nullptr;
}

bool MapData::removeObject(int tile_x, int tile_y) {
    Object* object = nullptr;
    int index = 0;
    for( auto cur_object : objects_ ) {
        Position cur_position = cur_object->tilePosition();
        if( cur_position.x == tile_x && cur_position.y == tile_y ) {
            object = cur_object;
            break;
        }
        index++;
    }
    if( object == nullptr ) {
        return false;
    }
    store(BasicItem(object->name()), object->tilePosition());
    object->releaseItems();
    objects_.erase(objects_.begin()+index);
    return true;
}

bool MapData::store(const BasicItem& item, Position tile_position, int occurrence) {
    Object* obj = getAssociatedChest(tile_position);
    if( obj == nullptr ) { // put item on the floor
        Tile& tile = this->tile(tile_position.x, tile_position.y);
        tile.addItem(item, occurrence);
        return false;
    }
    Chest* chest = static_cast<Chest*>(obj);
    int not_added = chest->addItem(item, occurrence);
    if( not_added > 0 ) {
        // put the item on the floor
        Tile& tile = this->tile(tile_position.x, tile_position.y);
        tile.addItem(item, not_added);
    }
    return true;
}

const Tile& MapData::tile(int x,int y) const {
    return tiles_[x+y*width_];
}

Tile& MapData::tile(int x,int y) {
    return tiles_[x+y*width_];
}

void MapData::createMap(MapData* data) {
    if( data == nullptr ) {
        Logger::error() << "cannot create map because data is nullptr" << Logger::endl;
        return;
    }
    int width = data->width();
    int height = data->height();
    srand(time(NULL)); // initialization of rand
    int seed = std::rand();//6224;
    float** noise_map = Noise::generateNoiseMap(width, height, seed, 150, 4, 0.5f, 2.f);

    Logger::info() << tr("Generating a new map of dimension: ") << width << " x " << height << Logger::endl;
    // create the background picture
    Biome* biome = new Biome("forest"); // generator is ownership of biome
    BackGroundGenerator generator(width,height,biome);
    std::string filename("_out.png");
    filename.insert(0, Utility::itos(seed));
    filename.insert(0, "_");
    filename.insert(0, biome->type());
    Logger::debug() << "Seed is " << seed << Logger::endl;
    generator.execute(filename, noise_map);

    data->setMapImageName(filename);

    Logger::info() << tr("Filling terrain...") << Logger::endl;
    for( int col=0; col < width; col++ ) {
        for( int row=0; row < height; row++ ) {
            Tile& tile = data->tile(col,row);
            float value = noise_map[col][row]* 255;
            int region = biome->getType( value ); // change btype according to 'value' given by Biome
            Tile::BType btype = Tile::GRASS;
            if( region == 0 ) {
                btype = Tile::WATER;
            } else if( region == 1 ) {
                btype = Tile::SAND;
            } else if( region == 2 ) {
                btype = Tile::DIRT;
            } else if( region == 4 ) {
                btype = Tile::ROCK;
            } else if( region == 5 ) {
                btype = Tile::COAL;
            } else if( region == 6 ) {
                btype = Tile::COPPER;
            } else if( region == 7 ) {
                btype = Tile::IRON;
            }
            tile.setBackgroundTile(tile.id(), btype);
            if( tile.background_type() >= Tile::ROCK || tile.background_type() == Tile::SAND ) {
                if( tile.background_type() >= Tile::ROCK ) { // coal copper iron
                    tile.setOccurrences( rand()%100+50 ); // valeur entre 50 et 150
                } else if( region != 2 ) {
                    tile.setOccurrences( rand()%40+10 ); // valeur entre 10 et 50
                }
            }
        }
    }
    Logger::info() << tr("End of generation") << Logger::endl;
}

/********************************************************************/

TileSetLib::TileSetLib() {
    tiles_surface_ = Utility::IMGLoad("images/tiles.png");
    walls_surface_ = Utility::IMGLoad("images/walls01.png");
    doors_surface_ = Utility::IMGLoad("images/doors01.png");
    grounds_surface_ = Utility::IMGLoad("images/grounds01.png");
    if( tiles_surface_ == nullptr || walls_surface_ == nullptr ) {
        Logger::error() << "cannot initialize TileSetLib" << Logger::endl;
    }
}

TileSetLib::~TileSetLib() {
    SDL_FreeSurface(tiles_surface_);
    SDL_FreeSurface(walls_surface_);
    SDL_FreeSurface(doors_surface_);
    SDL_FreeSurface(grounds_surface_);
}

TileSetLib* TileSetLib::instance() {
    if( singleton_ == nullptr ) {
        Logger::debug() << "creating TileSetLib singleton" << Logger::endl;
        singleton_ =  new TileSetLib();
    }
    return singleton_;
}

void TileSetLib::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        Logger::debug() << "destroying TileSetLib singleton" << Logger::endl;
        singleton_ = nullptr;
    }
}

/*!
 * \return the texture from the given \p tile.
 */
SDL_Texture* TileSetLib::getTextureFromTile(const Tile& tile, SDL_Renderer* renderer) {
    auto map_of_tiles = TileSetLib::instance()->mapOfTiles();
    auto map_of_walls = TileSetLib::instance()->mapOfWalls();
    auto map_of_doors = TileSetLib::instance()->mapOfDoors();
    auto map_of_grounds = TileSetLib::instance()->mapOfGrounds();
    int id = tile.id();
    int max = 5;

    bool background = (tile.cell_type() == Tile::EMPTY);

    if( tile.cell_type() == Tile::EMPTY ) {
        return nullptr;
    } else if( Tile::isFloor(tile) ) {
        id = int(tile.floor_type());
        background = true;
        if( map_of_grounds.find(id) != map_of_grounds.end()) {
            return map_of_grounds[id];
        }
    } else if( Tile::isWall(tile) ) {
        if(map_of_walls.find(id) != map_of_walls.end()) {
            return map_of_walls[id];
        }
        max = 4;
    } else if( Tile::isDoor(tile) ) {
        if(map_of_doors.find(id) != map_of_doors.end()) {
            return map_of_doors[id];
        }
        max = 4;
    } else {
        if(map_of_tiles.find(id) != map_of_tiles.end()) {
            return map_of_tiles[id];
        }
    }

    int x = id % max;
    int y = floor(id / max);
    SDL_Rect source;
    source.x = x * Utility::tileSize;
    source.y = y * Utility::tileSize;
    source.w = Utility::tileSize;
    source.h = Utility::tileSize;

    SDL_Surface* surf_dest = SDL_CreateRGBSurface(0, Utility::tileSize, Utility::tileSize, 32, 0, 0, 0, 0);

    SDL_Rect dest;
    dest.x = 0;
    dest.y = 0;
    dest.w = Utility::tileSize;
    dest.h = Utility::tileSize;

    SDL_Surface* surf_source = TileSetLib::instance()->tiles();
    if( Tile::isWall(tile) ) {
        surf_source = TileSetLib::instance()->walls();
    }
    if( Tile::isDoor(tile) ) {
        surf_source = TileSetLib::instance()->doors();
    }
    if( background ) {
        surf_source = TileSetLib::instance()->grounds();
    }

    SDL_BlitSurface(surf_source,
                    &source,
                    surf_dest,
                    &dest);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf_dest);
    if( background ) {
        TileSetLib::instance()->mapOfGrounds()[id] = texture;
    } else if( Tile::isWall(tile) ) {
        TileSetLib::instance()->mapOfWalls()[id] = texture;
    } else if( Tile::isDoor(tile) ) {
        TileSetLib::instance()->mapOfDoors()[id] = texture;
    } else {
        TileSetLib::instance()->mapOfTiles()[id] = texture;
    }
    return texture;
}

// Initialize singleton_ to nullptr
TileSetLib* TileSetLib::singleton_ = nullptr;

/********************************************************************/

GameBoard* GameBoard::cur_board = nullptr;

GameBoard::GameBoard(PeopleGroup* group, MapData* data, JobMgr* manager) : group_(group), data_(data), job_mgr_(manager) {
    cur_board = this;
}

GameBoard::~GameBoard() {
    cur_board = nullptr;
}

void GameBoard::animate(double delay_ms) {
    group_->animate(this, delay_ms);
    //data_->animate(delay_ms);
    for( auto object : data_->objects() ) {
        object->animate(delay_ms);
    }
    LoggerMgr::instance()->animate(delay_ms);
}

/********************************************************************/
