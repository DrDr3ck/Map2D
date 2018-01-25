#include "sdl_background.h"

#include <iostream>
#include <random>

/********************************************************************/

BackGroundGenerator::BackGroundGenerator(int width, int height) : width_(width), height_(height) {
    //int tilesize = 64;
    if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
        //window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_*tilesize, height_*tilesize, SDL_WINDOW_SHOWN);
        //renderer_ = SDL_CreateRenderer(window_, -1, 0);
        surface_ = IMG_Load("GrassGenerator72_01.png");
        //texture_ = SDL_CreateTextureFromSurface(renderer_, tiles_surface);
    }
}

BackGroundGenerator::~BackGroundGenerator() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
    if( renderer_ != nullptr ) {
        SDL_DestroyRenderer(renderer_);
    }
    if( window_ != nullptr ) {
        SDL_DestroyWindow(window_);
    }
}

void BackGroundGenerator::execute(const std::string& filename) const {
    if( surface_ == nullptr ) {
        std::cout << "Error: Cannot generate background without pictures" << std::endl;
        return;
    }

    int tilesize = 64;
    SDL_Surface* image = SDL_CreateRGBSurface(0, width_*tilesize, height_*tilesize, 32, 0, 0, 0, 0);
    int offset = 4; // left, right, top, bottom
    int fullsize = tilesize + offset*2;
    for( int col=0; col < width_; col++ ) {
        for( int row=0; row < height_; row++ ) {
            int idx = std::rand() % 4;
            SDL_Rect source;
            source.x = idx * fullsize;
            source.y = 0 * fullsize;
            source.w = fullsize;
            source.h = fullsize;

            SDL_Rect dest;
            dest.x = col * tilesize - offset;
            dest.y = row * tilesize - offset;
            dest.w = fullsize;
            dest.h = fullsize;
            SDL_BlitSurface(
                    surface_,
                    &source,
                    image,
                    &dest
            );


        }
    }

    IMG_SavePNG(image, filename.c_str());
}

/********************************************************************/
