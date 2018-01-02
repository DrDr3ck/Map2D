#include "map.h"

#include <iostream>
#include <cmath>

#include "SDL2/SDL_image.h"

/*************************/

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

/*************************/

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

const Tile& MapData::tile(int x,int y) const {
    return tiles_[x+y*width_];
}

Tile& MapData::tile(int x,int y) {
    return tiles_[x+y*width_];
}

/*************************/

TileSet::TileSet() {
    tiles_surface_ = SDL_LoadBMP("tiles.bmp");
    walls_surface_ = SDL_LoadBMP("walls01.bmp");
    if( tiles_surface_ == nullptr ) {
        std::cout << "cannot initialize TileSet" << std::endl;
    }
}

TileSet* TileSet::instance() {
    if( singleton_ == nullptr ) {
        std::cout << "creating singleton" << std::endl;
        singleton_ =  new TileSet();
    }
    return singleton_;
}

void TileSet::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        std::cout << "destroying singleton" << std::endl;
        singleton_ = nullptr;
    }
}

SDL_Texture* TileSet::getTextureFromTile(const Tile& tile, SDL_Renderer* renderer) {
    auto map_of_tiles = TileSet::instance()->mapOfTiles();
    auto map_of_walls = TileSet::instance()->mapOfWalls();
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

    SDL_Surface* surf_source = TileSet::instance()->tiles();
    if( tile.type() == Tile::WALL ) {
        surf_source = TileSet::instance()->walls();
    }

    SDL_BlitSurface(surf_source,
                    &source,
                    surf_dest,
                    &dest);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf_dest);
    if( tile.type() == Tile::WALL ) {
        TileSet::instance()->mapOfWalls()[id] = texture;
    } else {
        TileSet::instance()->mapOfTiles()[id] = texture;
    }
    return texture;
}

TileSet* TileSet::singleton_ = nullptr;
