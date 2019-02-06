#include "dialog.h"

#include "sdl_camera.h"

#include <tuple>
#include <SDL2/SDL.h>

/***********************************/

Dialog::Dialog(int x, int y, int width, int height) : View(), x_(x), y_(y), width_(width), height_(height) {
    surface_ = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_FillRect(surface_, NULL, SDL_MapRGBA(surface_->format, 255, 255, 255, 128));
}

Dialog::~Dialog() {
    SDL_FreeSurface(surface_);
}

void Dialog::setBackgroundColor(int R, int G, int B, int A) {
    SDL_FillRect(surface_, NULL, SDL_MapRGBA(surface_->format, R, G, B, A));
    SDL_Rect rect = {0,0,width_,20};
    SDL_FillRect(surface_, &rect, SDL_MapRGBA(surface_->format, R*0.9, G*0.9, B*0.9, A));
    rect = {width_ - 18, 2,16,16};
    SDL_FillRect(surface_, &rect, SDL_MapRGBA(surface_->format, 255, 100, 100, A));
}

void Dialog::do_render(Camera* camera, double delay_in_ms) {
    std::ignore = delay_in_ms;
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(main_renderer,surface_);
    SDL_Rect rect = {x_, y_, width_, height_ };
    sdl_camera->displayTexture(texture, &rect);
    SDL_DestroyTexture(texture);
}

void Dialog::handleEvent(Camera* camera) {
    std::ignore = camera;
}

/**************************************/

