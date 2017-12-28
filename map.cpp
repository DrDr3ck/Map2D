#include "map.h"

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
