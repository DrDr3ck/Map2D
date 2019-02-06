#include "dialog.h"

#include "sdl_camera.h"

#include <tuple>
#include <SDL2/SDL.h>

/***********************************/

Dialog::Dialog(int x, int y, int width, int height) : View(), x_(x), y_(y), width_(width), height_(height) {
    surface_ = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    setBackgroundColor(211,211,211,255);
}

Dialog::~Dialog() {
    SDL_FreeSurface(surface_);
}

void Dialog::setBackgroundColor(int R, int G, int B, int A) {
    red_ = R;
    green_ = G;
    blue_ = B;
    alpha_ = A;
    SDL_FillRect(surface_, NULL, SDL_MapRGBA(surface_->format, 0, 0, 0, A));
    SDL_Rect rect = {1,20,width_-2,height_-21};
    SDL_FillRect(surface_, &rect, SDL_MapRGBA(surface_->format, R, G, B, A));
    SDL_Rect window_rect = {1,1,width_-2,18};
    SDL_FillRect(surface_, &window_rect, SDL_MapRGBA(surface_->format, R*0.9, G*0.9, B*0.9, A));
    SDL_Rect kill_rect = {width_ - 18, 2,16,16};
    SDL_FillRect(surface_, &kill_rect, SDL_MapRGBA(surface_->format, 255, 100, 100, A));
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

/*!
 * \return true if this dialog has the focus
 * (i.e. if mouse cursor is over the dialog).
 */
bool Dialog::hasFocus(int mouse_x, int mouse_y) {
    if( mouse_x < x_ ) return false;
    if( mouse_x > x_+width_ ) return false;
    if( mouse_y < y_ ) return false;
    if( mouse_y > y_+height_ ) return false;
    return true;
}

void Dialog::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    SDL_Rect kill_rect = {width_ - 18, 2,16,16};
    SDL_Rect window_rect = {1,1,width_-2,18};
    const SDL_Event& event = sdl_camera->event();
    int rel_mouse_x = sdl_camera->mouse_x() - x_;
    int rel_mouse_y = sdl_camera->mouse_y() - y_;
    switch( event.type ) {
        case SDL_MOUSEBUTTONDOWN:
            if( event.button.button == SDL_BUTTON_LEFT ) {
                if( Utility::contains(kill_rect, rel_mouse_x, rel_mouse_y) ) {
                    // kill this dialog !!
                    kill_me_ = true;
                } else if( Utility::contains(window_rect, rel_mouse_x, rel_mouse_y) ) {
                    grabbing_ = true;
                    rel_grab_x_ = rel_mouse_x;
                    rel_grab_y_ = rel_mouse_y;
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if( event.button.button == SDL_BUTTON_LEFT ) {
                grabbing_ = false;
            }
            break;
        case SDL_MOUSEMOTION:
            //onMouseMove(event.motion.x, event.motion.y);
            if( grabbing_ ) {
                x_ = x_ + rel_mouse_x - rel_grab_x_;
                y_ = y_ + rel_mouse_y - rel_grab_y_;
            }
            break;
        default:
            break;
    }
}

/**************************************/

