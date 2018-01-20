#include "map.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "SDL2/SDL_image.h"

/********************************************************************/

/*!
 * Transforms an integer into a string.
 */
std::string Utility::itos(int i) {
    std::ostringstream stm;
    stm << i ;
    return stm.str();
}

/********************************************************************/

Tile::Tile(int id, Type type) : id_(id), type_(type) {
}

Tile::~Tile() {
}

int Tile::id() const {
    return id_;
}

Tile::Type Tile::type() const {
    return type_;
}

void Tile::setTile(int id, Type type) {
    id_ = id;
    type_ = type;
}

std::string Tile::typeTileToString(Tile::Type type) {
    if( type == Tile::BLOCK ) return "Block";
    if( type == Tile::DOOR ) return "Door";
    if( type == Tile::WALL ) return "Wall";
    if( type == Tile::EMPTY ) return "Nothing...";
    std::cout << "unable to find string for type: " << type << std::endl;
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
    int id = 0;
    if( x > 0 ) {
        if( tile(x-1,y).type() == Tile::WALL ) {
            id += 1;
            Tile& tile_x_1 = tile(x-1,y);
            tile_x_1.setTile( tile_x_1.id() + 4, Tile::WALL);
        }
    }
    if( x < width_-1 ) {
        if( tile(x+1,y).type() == Tile::WALL ) {
            id += 4;
            Tile& tile_x_1 = tile(x+1,y);
            tile_x_1.setTile( tile_x_1.id() + 1, Tile::WALL);
        }
    }
    if( y < height_-1 ) {
        if( tile(x,y+1).type() == Tile::WALL ) {
            id += 2;
            Tile& tile_y_1 = tile(x,y+1);
            tile_y_1.setTile( tile_y_1.id() + 8, Tile::WALL);
        }
    }
    if( y > 0 ) {
        if( tile(x,y-1).type() == Tile::WALL ) {
            id += 8;
            Tile& tile_y_1 = tile(x,y-1);
            tile_y_1.setTile( tile_y_1.id() + 2, Tile::WALL);
        }
    }
    tile(x,y).setTile(id, Tile::WALL);
}

void MapData::removeWall(int x, int y) {
    tile(x,y).setTile(0, Tile::EMPTY);
    if( x > 0 ) {
        if( tile(x-1,y).type() == Tile::WALL ) {
            Tile& tile_x_1 = tile(x-1,y);
            tile_x_1.setTile( tile_x_1.id() - 4, Tile::WALL);
        }
    }
    if( x < width_-1 ) {
        if( tile(x+1,y).type() == Tile::WALL ) {
            Tile& tile_x_1 = tile(x+1,y);
            tile_x_1.setTile( tile_x_1.id() - 1, Tile::WALL);
        }
    }
    if( y < height_-1 ) {
        if( tile(x,y+1).type() == Tile::WALL ) {
            Tile& tile_y_1 = tile(x,y+1);
            tile_y_1.setTile( tile_y_1.id() - 8, Tile::WALL);
        }
    }
    if( y > 0 ) {
        if( tile(x,y-1).type() == Tile::WALL ) {
            Tile& tile_y_1 = tile(x,y-1);
            tile_y_1.setTile( tile_y_1.id() - 2, Tile::WALL);
        }
    }
}

const Tile& MapData::tile(int x,int y) const {
    return tiles_[x+y*width_];
}

Tile& MapData::tile(int x,int y) {
    return tiles_[x+y*width_];
}

/********************************************************************/

TileSetLib::TileSetLib() {
    tiles_surface_ = IMG_Load("tiles.png");
    walls_surface_ = IMG_Load("walls01.png");
    if( tiles_surface_ == nullptr || walls_surface_ == nullptr ) {
        std::cout << "cannot initialize TileSetLib" << std::endl;
    }
}

TileSetLib::~TileSetLib() {
    SDL_FreeSurface(tiles_surface_);
    SDL_FreeSurface(walls_surface_);
}

TileSetLib* TileSetLib::instance() {
    if( singleton_ == nullptr ) {
        std::cout << "creating TileSetLib singleton" << std::endl;
        singleton_ =  new TileSetLib();
    }
    return singleton_;
}

void TileSetLib::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        std::cout << "destroying TileSetLib singleton" << std::endl;
        singleton_ = nullptr;
    }
}

/*!
 * \return the texture from the given \p tile.
 */
SDL_Texture* TileSetLib::getTextureFromTile(const Tile& tile, SDL_Renderer* renderer) {
    auto map_of_tiles = TileSetLib::instance()->mapOfTiles();
    auto map_of_walls = TileSetLib::instance()->mapOfWalls();
    int id = tile.id();
    int max = 5;
    if( tile.type() == Tile::WALL ) {
        if(map_of_walls.find(id) != map_of_walls.end()) {
            return map_of_walls[id];
        }
        max = 4;
    } else {
        if(map_of_tiles.find(id) != map_of_tiles.end()) {
            return map_of_tiles[id];
        }
    }

    static int tileSize = 64;
    int x = id % max;
    int y = floor(id / max);
    SDL_Rect source;
    source.x = x * tileSize;
    source.y = y * tileSize;
    source.w = tileSize;
    source.h = tileSize;

    SDL_Surface* surf_dest = SDL_CreateRGBSurface(0, tileSize, tileSize, 32, 0, 0, 0, 0);

    SDL_Rect dest;
    dest.x = 0;
    dest.y = 0;
    dest.w = tileSize;
    dest.h = tileSize;

    SDL_Surface* surf_source = TileSetLib::instance()->tiles();
    if( tile.type() == Tile::WALL ) {
        surf_source = TileSetLib::instance()->walls();
    }

    SDL_BlitSurface(surf_source,
                    &source,
                    surf_dest,
                    &dest);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf_dest);
    if( tile.type() == Tile::WALL ) {
        TileSetLib::instance()->mapOfWalls()[id] = texture;
    } else {
        TileSetLib::instance()->mapOfTiles()[id] = texture;
    }
    return texture;
}

// Initialize singleton_ to nullptr
TileSetLib* TileSetLib::singleton_ = nullptr;

/********************************************************************/
