#include "sdl_background.h"
#include "perlin_noise.h"
#include "xml_document.h"
#include "translator.h"
#include "logger.h"
#include "utility.h"

#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>

/********************************************************************/

bool BiomeReader::readBiome(Biome& biome) {
    std::string filename("biomes.xml");
    XMLNode* doc = XMLDocument::read_doc(filename);
    if( doc == nullptr ) {
        Logger::error() << tr("Cannot read file ") << filename << Logger::endl;
        return false;
    }
    XMLNode* biome_node = doc->getNodeFromName(biome.type());
    if( biome_node == nullptr ) {
        Logger::error() << tr("Cannot find biome of type ") << biome.type() << Logger::endl;
        return false;
    }
    for( auto node : biome_node->nodes() ) {
        std::string name = node->name();
        XMLAttr* min_attr = node->getAttrFromName("min");
        XMLAttr* max_attr = node->getAttrFromName("max");
        if( min_attr == nullptr || max_attr == nullptr ) {
            Logger::error() << "Type " << name << " of biome " << biome.type() << " is not correctly defined, check min and max attributes" << Logger::endl;
            continue;
        }
        int min = atoi(min_attr->value().c_str());
        int max = atoi(max_attr->value().c_str());
        if( max == 0 ) {
            Logger::error() << "Suspicious max value for type " << name << " of biome " << biome.type() << ", check if value is a valid integer" << Logger::endl;
            continue;
        }
        std::string surface_name("generator/");
        surface_name.append(biome.type()).append("_generator72_").append(name).append(".png");
        std::cout << surface_name << min << max << std::endl;
        //SDL_Surface* surface = IMG_Load("generator/WaterGenerator72_01.png");
    }

    return true;
}

/********************************************************************/

Biome::Biome(const std::string& type) : type_(type) {
    bool result = BiomeReader::readBiome(*this);
    if( !result ) { // if fail, get this dummy biome
        Logger::error() << tr("Using default biome of type forest") << Logger::endl;
        SDL_Surface* water_surface = IMG_Load("generator/forest_generator72_water.png");
        if( water_surface != nullptr ) {
            addSurfaceHeight(water_surface, 20);
        }
        SDL_Surface* sand_surface = IMG_Load("generator/forest_generator72_sand.png");
        if( sand_surface != nullptr ) {
            addSurfaceHeight(sand_surface, 50);
        }
        SDL_Surface* grass_surface = IMG_Load("generator/forest_generator72_grass.png");
        if( grass_surface != nullptr ) {
            addSurfaceHeight(grass_surface, 220);
        }
        SDL_Surface* rock_surface = IMG_Load("generator/forest_generator72_rock.png");
        if( rock_surface != nullptr ) {
            addSurfaceHeight(rock_surface, 260);
        }
        SDL_Surface* coal_surface = IMG_Load("generator/CoalGenerator72_01.png");
        if( coal_surface != nullptr ) {
            addSurfaceHeight(coal_surface, 261);
        }
        SDL_Surface* copper_surface = IMG_Load("generator/CopperGenerator72_01.png");
        if( copper_surface != nullptr ) {
            addSurfaceHeight(copper_surface, 262);
        }
        SDL_Surface* iron_surface = IMG_Load("generator/IronGenerator72_01.png");
        if( iron_surface != nullptr ) {
            addSurfaceHeight(iron_surface, 263);
        }
    }

    const int enlargedtilesize = 72;
    for( auto surf : surfaces_ ) {
        int column = surf->w / enlargedtilesize;
        int row = surf->h / enlargedtilesize;
        surface_columns_.push_back(column);
        surface_rows_.push_back(row);
    }
}

Biome::~Biome() {
    for( auto surf : surfaces_ ) {
        if( surf != nullptr ) {
            SDL_FreeSurface(surf);
        }
    }
}

void Biome::addSurfaceHeight(SDL_Surface* surface, int height) {
    surfaces_.push_back(surface);
    heights_.push_back(height);
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
    Logger::debug() << "Sorting map tiles" << Logger::endl;
    sort(map_tiles.begin(), map_tiles.end(), wayToSortTile);

    for( unsigned int index=0; index < map_tiles.size(); index++ ) {
        int col = map_tiles[index].x;
        int row = map_tiles[index].y;

        int type = biome_->getType(map_tiles[index].value);
        // if type is rock: it may change into coal/copper/iron
        if( type == 3 ) { // rock
            // type 4 = coal, type 5 = copper, type 6 = iron
            int proba = std::rand() % 100;
            if( proba < 10 ) {
                type = 4;
                noise_map[col][row] = 1.023; // 261 / 255
            } else if( proba < 15 ) {
                type = 5;
                noise_map[col][row] = 1.027;
            } else if( proba < 20 ) {
                type = 6;
                noise_map[col][row] = 1.031;
            }
        }
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

    Logger::debug() << "Saving map background in " << filename << Logger::endl;

    IMG_SavePNG(image, filename.c_str());
}

/********************************************************************/
