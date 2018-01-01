#include "map.h"

#include <iostream>

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

TileManager::TileManager() {
    tiles_ = SDL_LoadBMP("tiles.bmp");
    if( tiles_ == nullptr ) {
        std::cout << "cannot initialize TileManager" << std::endl;
    }
}

TileManager* TileManager::instance() {
    if( singleton_ == nullptr ) {
        std::cout << "creating singleton" << std::endl;
        singleton_ =  new TileManager();
    }
    return singleton_;
}

void TileManager::kill () {
    if( singleton_ != nullptr ) {
        delete singleton_;
        std::cout << "destroying singleton" << std::endl;
        singleton_ = nullptr;
    }
}

SDL_Texture* TileManager::getTextureFromTileId(int id, SDL_Renderer* renderer) {
    static int tileSize = 64;
    SDL_Rect source;
    source.x = 1 * tileSize;
    source.y = 1 * tileSize;
    source.w = tileSize;
    source.h = tileSize;

    SDL_Surface* surf_dest = SDL_CreateRGBSurface(0, tileSize, tileSize, 32, 0, 0, 0, 0);

    SDL_Rect dest;
    dest.x = 0;
    dest.y = 0;
    dest.w = tileSize;
    dest.h = tileSize;

    SDL_Surface* surf_source = TileManager::instance()->tiles();

    SDL_BlitSurface(surf_source,
                    &source,
                    surf_dest,
                    &dest);

    return SDL_CreateTextureFromSurface(renderer, surf_dest);
}

TileManager* TileManager::singleton_ = nullptr;
