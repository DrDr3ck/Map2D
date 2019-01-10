#include "sdl_background.h"
#include "perlin_noise.h"

#include <iostream>
#include <sstream>
#include <random>

/********************************************************************/

BackGroundGenerator::BackGroundGenerator(int width, int height) : width_(width), height_(height), window_(nullptr), renderer_(nullptr) {
    //int tilesize = 64;
    if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
        //window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_*tilesize, height_*tilesize, SDL_WINDOW_SHOWN);
        //renderer_ = SDL_CreateRenderer(window_, -1, 0);
        SDL_Surface* water_surface = IMG_Load("WaterGenerator72_01.png");
        if( water_surface != nullptr ) {
            surfaces_.push_back(water_surface);
            heights_.push_back(20);
        }
        SDL_Surface* sand_surface = IMG_Load("SandGenerator72_01.png");
        if( sand_surface != nullptr ) {
            surfaces_.push_back(sand_surface);
            heights_.push_back(50);
        }
        SDL_Surface* grass_surface = IMG_Load("GrassGenerator72_02.png");
        if( grass_surface != nullptr ) {
            surfaces_.push_back(grass_surface);
            heights_.push_back(220);
        }
        SDL_Surface* rock_surface = IMG_Load("RockGenerator72_01.png");
        if( rock_surface != nullptr ) {
            surfaces_.push_back(rock_surface);
            heights_.push_back(260);
        }
        //texture_ = SDL_CreateTextureFromSurface(renderer_, tiles_surface);
    }
}

BackGroundGenerator::~BackGroundGenerator() {
    for( auto surf : surfaces_ ) {
        if( surf != nullptr ) {
            SDL_FreeSurface(surf);
        }
    }
    if( renderer_ != nullptr ) {
        SDL_DestroyRenderer(renderer_);
    }
    if( window_ != nullptr ) {
        SDL_DestroyWindow(window_);
    }
}

std::string itos(int i){
    std::stringstream ss;
    ss<<i;
    return ss.str();
}

int BackGroundGenerator::getType(float value) const {
    for( int region = 0; region < int(heights_.size()); region++ ) {
        if( value <= heights_[region] ) {
            return region;
        }
    }
    return heights_.size()-1;
}

void BackGroundGenerator::execute(const std::string& filename) const {
    if( surfaces_.size() == 0 ) {
        std::cout << "Error: Cannot generate background without pictures" << std::endl;
        return;
    }
    int enlargedtilesize = 72;
    std::vector<int> columns;
    std::vector<int> rows;
    for( auto surf : surfaces_ ) {
        int column = surf->w / enlargedtilesize;
        int row = surf->h / enlargedtilesize;
        columns.push_back(column);
        rows.push_back(row);
    }

    int tilesize = 64;
    Uint32 amask = 0x000000ff;
    SDL_Surface* image = SDL_CreateRGBSurface(0, width_*tilesize, height_*tilesize, 32, 0, 0, 0, amask);
    int offset = 4; // left, right, top, bottom
    int fullsize = tilesize + offset*2;

    //int type = 2; // TODO : terrain type (depend of the value in the perlin noise generated map)
    float** noise_map = Noise::generateNoiseMap(width_, height_, rand(), 150, 4, 0.5f, 2.f);

    for( int col=0; col < width_; col++ ) {
        std::string str_row = "";
        for( int row=0; row < height_; row++ ) {
            int type = getType(noise_map[col][row]* 255);
            int idx = std::rand() % columns[type];
            int idy = std::rand() % rows[type];
            SDL_Rect source;
            source.x = idx * fullsize;
            source.y = idy * fullsize;
            source.w = fullsize;
            source.h = fullsize;

            str_row.append(itos(idx+idy*8)).append(std::string(" "));

            SDL_Rect dest;
            dest.x = col * tilesize - offset;
            dest.y = row * tilesize - offset;
            dest.w = fullsize;
            dest.h = fullsize;
            SDL_BlitSurface(
                    surfaces_[type],
                    &source,
                    image,
                    &dest
            );


        }
        //std::cout << str_row << std::endl;
    }

    IMG_SavePNG(image, filename.c_str());
}

/********************************************************************/
