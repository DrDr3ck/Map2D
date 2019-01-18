#include "sdl_background.h"
#include "perlin_noise.h"
#include "logger.h"
#include "utility.h"

#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>

/********************************************************************/

Biome::Biome(const std::string& type) {
    if( type == "forest" ) {
        SDL_Surface* water_surface = IMG_Load("WaterGenerator72_01.png");
        if( water_surface != nullptr ) {
            surfaces_.push_back(water_surface);
            heights_.push_back(20);
        }
        SDL_Surface* sand_surface = IMG_Load("SandGenerator72_02.png");
        if( sand_surface != nullptr ) {
            surfaces_.push_back(sand_surface);
            heights_.push_back(50);
        }
        SDL_Surface* grass_surface = IMG_Load("GrassGenerator72_02.png");
        if( grass_surface != nullptr ) {
            surfaces_.push_back(grass_surface);
            heights_.push_back(220);
        }
        SDL_Surface* rock_surface = IMG_Load("RockGenerator72_03.png");
        if( rock_surface != nullptr ) {
            surfaces_.push_back(rock_surface);
            heights_.push_back(260);
        }
    }

    int enlargedtilesize = 72;
    for( auto surf : surfaces_ ) {
        int column = surf->w / enlargedtilesize;
        int row = surf->h / enlargedtilesize;
        surface_columns_.push_back(column);
        surface_columns_.push_back(row);
    }
}

Biome::~Biome() {
    for( auto surf : surfaces_ ) {
        if( surf != nullptr ) {
            SDL_FreeSurface(surf);
        }
    }
}

int Biome::getType(float value) const {
    for( int region = 0; region < int(heights_.size()); region++ ) {
        if( value <= heights_[region] ) {
            return region;
        }
    }
    return heights_.size()-1;
}

bool Biome::is_valid() const {
    if( surfaces_.size() == 0 ) {
        Logger::error() << "Cannot generate background without pictures" << Logger::endl;
        return false;
    }
    return true;
}

/********************************************************************/

BackGroundGenerator::BackGroundGenerator(
    int width, int height, Biome* biome
) : width_(width), height_(height), biome_(biome) {
    if( biome_ == nullptr ) {
        biome_ = new Biome("forest"); // Default biome
    }
}

BackGroundGenerator::~BackGroundGenerator() {
}

namespace {
    struct MapTile {
        float value; // between 0 and 255
        int x;
        int y;
    };

    bool wayToSortTile(MapTile i, MapTile j) {
        return i.value < j.value;
    }
}

void BackGroundGenerator::execute(const std::string& filename, float** noise_map) const {
    if( !biome_->is_valid() ) {
        Logger::error() << "biome " << biome_->type() << " is invalid" << Logger::endl;
        return;
    }

    const std::vector<int>& columns = biome_->surface_columns();
    const std::vector<int>& rows = biome_->surface_rows();

    Uint32 amask = 0x000000ff;
    SDL_Surface* image = SDL_CreateRGBSurface(0, width_*Utility::tileSize, height_*Utility::tileSize, 32, 0, 0, 0, amask);
    int offset = 4; // left, right, top, bottom
    int fullsize = Utility::tileSize + offset*2;

    if( noise_map == nullptr ) {
        noise_map = Noise::generateNoiseMap(width_, height_, rand(), 150, 4, 0.5f, 2.f);
    }

    std::vector<MapTile> map_tiles;
    for( int col=0; col < width_; col++ ) {
        for( int row=0; row < height_; row++ ) {
            float value = noise_map[col][row]* 255;
            MapTile tile;
            tile.value = value;
            tile.x = col; tile.y = row;
            map_tiles.push_back(tile);
        }
    }
    sort(map_tiles.begin(), map_tiles.end(), wayToSortTile);

    for( unsigned int index=0; index < map_tiles.size(); index++ ) {
        int type = biome_->getType(map_tiles[index].value);
        int col = map_tiles[index].x;
        int row = map_tiles[index].y;
        int idx = std::rand() % columns[type];
        int idy = std::rand() % rows[type];
        SDL_Rect source;
        source.x = idx * fullsize;
        source.y = idy * fullsize;
        source.w = fullsize;
        source.h = fullsize;

        SDL_Rect dest;
        dest.x = col * Utility::tileSize - offset;
        dest.y = row * Utility::tileSize - offset;
        dest.w = fullsize;
        dest.h = fullsize;
        SDL_BlitSurface(
                biome_->surface(type),
                &source,
                image,
                &dest
        );
    }

    IMG_SavePNG(image, filename.c_str());
}

/********************************************************************/
