#include "camera.h"

#include <iostream>

MapView::MapView(MapData* data) : data_(data) {
}

void MapView::do_render(SDL_Renderer* main_renderer) {
    if( background_ == nullptr ) {
        SDL_Surface* bg_surface = SDL_LoadBMP("background.bmp");
        background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
    }
    SDL_RenderCopy(main_renderer, background_, NULL, NULL);
}

void MapView::handleEvent(const SDL_Event& /*event*/) {

}

/***********************************/

Camera::Camera() {
    window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    main_renderer_ = SDL_CreateRenderer(window_, -1, 0);
    pause_ = true;
    quit_ = false;
}

Camera::~Camera() {
    // Quit SDL
    SDL_DestroyWindow(window_);
}

void Camera::addView(View* view) {
    views_.push_back(view);
}

void Camera::removeView(View* view) {
    views_.remove(view);
}

void Camera::render() {
    for( auto view : views_ ) {
        view->do_render(main_renderer_);
    }
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

void Camera::handleEvent(const SDL_Event& event) {
    if( event.type == SDL_KEYDOWN ) {
        if( event.key.keysym.sym == SDLK_SPACE ) {
            pause_ = !pause_;
        } else if( event.key.keysym.sym == SDLK_ESCAPE ) {
            quit_ = true;
        }
    }
    for( auto view : views_ ) {
        view->handleEvent(event);
    }
}
