#include "sdl_button.h"

#include <iostream>
#include <math.h>
#include <string>

#include "sdl_camera.h"
#include "sdl_tool.h"

/********************************************************************/

bool SDLButtonManager::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    const SDL_Event& e = sdl_camera->event();
    bool button_pressed = false;
    if( e.type == SDL_MOUSEBUTTONDOWN ) {
        button_pressed = true;
    }
    for( auto button : buttons_ ) {
        // get mouse position
        if( !button->isVisible() ) {
            continue;
        }
        if( button->mouseOverButton(camera->mouse_x(), camera->mouse_y()) ) {
            button->hasFocus(true);
            if( button_pressed ) {
                if( button->isActive() ) {
                    button->deactivate();
                } else {
                    button->activate();
                }
            }
        } else {
            button->hasFocus(false);
            if( button_pressed ) {
                button->deactivate();
            }
        }
    }
    return false;
}

// remark: the delay_in_ms may be use to display animated buttons
void SDLButtonManager::do_render(Camera* camera, double /*delay_in_ms*/) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    // iterate over all visible buttons, get texture and display them
    for( auto menu : menus_ ) {
        if( !menu->isVisible() ) {
            continue;
        }
        SDL_Rect rect = {menu->x(), menu->y(), menu->width(), menu->height()};
        SDL_SetRenderDrawColor( main_renderer, 200, 200, 200, 125 );
        SDL_RenderFillRect(main_renderer, &rect);
        SDL_SetRenderDrawColor( main_renderer, 50, 50, 50, 255 );
        SDL_RenderDrawRect(main_renderer, &rect);
    }
    for( auto b : buttons_ ) {
        if( !b->isVisible() ) {
            continue;
        }
        SDLButton* button = dynamic_cast<SDLButton*>(b);
        SDL_Texture* texture = button->getTexture(main_renderer);
        sdl_camera->displayTexture(texture, &button->rect());

        // display white rectangle if button has focus
        if( button->hasFocus() ) {
            SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
            SDL_RenderDrawRect(main_renderer, &button->rect());
        }
    }
}

/********************************************************************/

SDLButton::SDLButton(std::string name, int x, int y) : Button(name,x,y) {
    surface_ = Utility::IMGLoad(name.c_str());
    int w = surface_->w;
    int h = surface_->h;
    setSize(w,h);
    rect_ = {x,y,w,h};
    texture_ = nullptr;
}

SDLButton::~SDLButton() {
    SDL_FreeSurface(surface_);
    if( texture_ != nullptr )  {
        SDL_DestroyTexture(texture_);
    }
}

SDL_Texture* SDLButton::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
    }
    return texture_;
}

void SDLButton::activate() {
    Button::activate();
}

void SDLButton::deactivate() {
    Button::deactivate();
}

void SDLButton::setSize(int w, int h) {
    Button::setSize(w,h);
    rect_ = {x(),y(),this->w(),this->h()};
}

void SDLButton::setPosition(int x,int y) {
    Button::setPosition(x,y);
    rect_ = {this->x(),this->y(),w(),h()};
}

/********************************************************************/

void SDLButtonMenu::activate() {
    SDLButton::activate();
    menu_->show();
}

void SDLButtonMenu::deactivate() {
    menu_->hide();
    SDLButton::deactivate();
}

void SDLQuitButton::activate() {
    camera_->set_quit();
}

/********************************************************************/

SDLToolButton::SDLToolButton(
    SDLTool* tool, std::string icon_name, int x, int y
) : SDLButton(icon_name, x, y), tool_(tool){
    int old_w = std::max(w(),64);
    int old_h = std::max(h(),64);
    setSize(64,64); // size of the button in the menu
    rect_ = {x,y,old_w,old_h}; // size of the tool on the map
    tool_->setRect( rect_ );
}

SDLToolButton::~SDLToolButton() {
    delete tool_;
}

void SDLToolButton::activate() {
    tool_->activate();
}

void SDLToolButton::deactivate() {
    tool_->deactivate();
}

/********************************************************************/
