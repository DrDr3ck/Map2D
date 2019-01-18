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

/********************************************************************/

MapData::MapData(int width, int height) : width_(width), height_(height) {
    tiles_ = new Tile[width*height];
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

void MapData::addObject(Object* object, int x, int y) {
    PositionObject position_object;
    position_object.x = x;
    position_object.y = y;
    position_object.object = object;
    objects_.push_back( position_object );
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
    int seed = rand();
    float** noise_map = Noise::generateNoiseMap(width, height, seed, 150, 4, 0.5f, 2.f);

    // create the background picture
    Biome* biome = new Biome("forest"); // generator is ownership of biome
    BackGroundGenerator generator(width,height,biome);
    std::string filename("_out.png");
    filename.insert(0, Utility::itos(seed));
    generator.execute(filename);

    for( int col=0; col < width; col++ ) {
        for( int row=0; row < height; row++ ) {
            Tile& tile = data->tile(col,row);
            float value = noise_map[col][row]* 255;
            int region = biome->getType( value );
            // TODO change btype according to 'value' (need to be given by Biome ?)
            Tile::BType btype = Tile::GRASS;
            if( region == 0 ) {
                btype = Tile::WATER;
            } else if( region == 1 ) {
                btype = Tile::SAND;
            } else if( region == 3 ) {
                btype = Tile::ROCK;
            }
            tile.setBackgroundTile(tile.id(), btype);
            if( tile.background_type() == Tile::ROCK ) {
                tile.setOccurrences( rand()%40+10 ); // valeur entre 10 et 50
            }
        }
    }
}

/********************************************************************/

TileSetLib::TileSetLib() {
    tiles_surface_ = Utility::IMGLoad("tiles.png");
    walls_surface_ = Utility::IMGLoad("walls01.png");
    grounds_surface_ = Utility::IMGLoad("grounds01.png");
    if( tiles_surface_ == nullptr || walls_surface_ == nullptr ) {
        Logger::error() << "cannot initialize TileSetLib" << Logger::endl;
    }
}

TileSetLib::~TileSetLib() {
    SDL_FreeSurface(tiles_surface_);
    SDL_FreeSurface(walls_surface_);
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
    } else {
        TileSetLib::instance()->mapOfTiles()[id] = texture;
    }
    return texture;
}

// Initialize singleton_ to nullptr
TileSetLib* TileSetLib::singleton_ = nullptr;

/********************************************************************/

GameBoard::GameBoard(PeopleGroup* group, MapData* data, JobMgr* manager) : group_(group), data_(data), job_mgr_(manager) {
    // debug
    // add chest for fun :)
    data->addObject(new Chest(), 2, 3);
    // end debug
}

GameBoard::~GameBoard() {
}

void GameBoard::animate(double delay_ms) {
    group_->animate(this, delay_ms);
    //data_->animate(delay_ms);
    LoggerMgr::instance()->animate(delay_ms);
}

/********************************************************************/
