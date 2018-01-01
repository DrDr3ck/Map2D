#include "sdl_camera.h"

#include <iostream>

MapView::MapView(MapData* data) : data_(data) {
}

void MapView::do_render(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    if( background_ == nullptr ) {
        SDL_Surface* bg_surface = SDL_LoadBMP("background.bmp");
        background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
    }
    SDL_RenderCopy(main_renderer, background_, NULL, NULL);
}

void MapView::handleEvent(Camera* camera) {
    if( camera == nullptr ) return;
}

/***********************************/

SDLCamera::SDLCamera() : Camera() {
    window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    main_renderer_ = SDL_CreateRenderer(window_, -1, 0);
}

SDLCamera::~SDLCamera() {
    // Quit SDL
    SDL_DestroyWindow(window_);
    do_quit();
}

bool SDLCamera::valid() const {
    return window_ != nullptr;
}

void SDLCamera::render() {
    Camera::render();
    if( pause_ ) {
        SDL_SetRenderDrawColor( main_renderer_, 222, 50, 50, 255 );
        SDL_Rect r;
        r.x = 370;
        r.y = 250;
        r.w = 20;
        r.h = 50;
        SDL_RenderFillRect( main_renderer_, &r );
        r.x = 400;
        SDL_RenderFillRect( main_renderer_, &r );
    }
    SDL_RenderPresent(main_renderer_);
}

void SDLCamera::handleEvent() {
    SDL_PollEvent(&event_);
    if(event_.type == SDL_QUIT) {
        quit_ = true;
        return;
    }
    if( event_.type == SDL_KEYDOWN ) {
        if( event_.key.keysym.sym == SDLK_SPACE ) {
            pause_ = !pause_;
        } else if( event_.key.keysym.sym == SDLK_ESCAPE ) {
            quit_ = true;
        }
    }
    Camera::handleEvent();
}

void SDLCamera::do_quit() const {
    if( !valid() ) {
        std::cout << "Camera not valid: " << SDL_GetError() << std::endl;
    }
    SDL_Quit();
}
