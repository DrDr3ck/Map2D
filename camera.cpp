#include "camera.h"

MapView::MapView(MapData* data) : data_(data) {
}

void MapView::do_render(SDL_Renderer* main_renderer) {
    if( background_ == nullptr ) {
        SDL_Surface* bg_surface = SDL_LoadBMP("background.bmp");
        background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
    }
    SDL_RenderCopy(main_renderer, background_, NULL, NULL);
    SDL_RenderPresent(main_renderer);
}

/***********************************/

Camera::Camera() {
    window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    main_renderer_ = SDL_CreateRenderer(window_, -1, 0);
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
}
