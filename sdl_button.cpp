#include "sdl_button.h"
#include "font.h"

#include <iostream>
#include <math.h>
#include <string>

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
                    if( connections_.find(button) != connections_.end()) {
                        (*connections_[button])();
                    }
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
        sdl_camera->displayButton(button);

        // display white rectangle if button has focus
        if( button->hasFocus() ) {
            SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
            SDL_RenderDrawRect(main_renderer, &button->rect());
        }
    }
}

/********************************************************************/

SDLButton::SDLButton(const std::string& icon_name, int x, int y) : Button(icon_name,x,y) {
    surface_ = Utility::IMGLoad(icon_name.c_str());
    int w = surface_->w;
    int h = surface_->h;
    setSize(w,h);
    rect_ = {x,y,w,h};
    texture_ = nullptr;
}

SDLButton::SDLButton(const std::string& icon_name, const std::string& text, int x, int y) : Button(icon_name,x,y) {
    surface_ = Utility::IMGLoad(icon_name.c_str());
    int w = surface_->w;
    int h = surface_->h;
    setSize(w,h);
    rect_ = {x,y,w,h};
    texture_ = nullptr;
    text_ = text;
}

SDLButton::SDLButton(int x, int y) : Button("none",x,y) {
    surface_ = nullptr;
    rect_ = {x,y,1,1};
    texture_ = nullptr;
}

SDLButton::~SDLButton() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
    if( texture_ != nullptr )  {
        SDL_DestroyTexture(texture_);
    }
}

void SDLButton::setIcon(const std::string& icon_name) {
    if( icon_name == buttonName() ) {
        return; // icon is already set
    }
    setButtonName(icon_name);
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
    }
    if( texture_ != nullptr )  {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    surface_ = Utility::IMGLoad(icon_name.c_str());
    int w = surface_->w;
    int h = surface_->h;
    setSize(w,h);
    rect_.w = w;
    rect_.h = h;
}

SDL_Texture* SDLButton::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        // Transparency with green color
        Uint32 key = SDL_MapRGB(surface_->format, 0, 255, 0);
        SDL_SetColorKey(surface_ , SDL_TRUE, key);
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
    }
    return texture_;
}

void SDLButton::setTexture(SDL_Texture* texture) {
    if( texture == nullptr ) return;
    if( texture_ != nullptr ) {
        SDL_DestroyTexture(texture_);
    }
    texture_ = texture;
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    setSize(w,h);
    rect_.w = w;
    rect_.h = h;
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

/********************************************************************/

void SDLQuitButton::activate() {
    camera_->set_quit();
}

/********************************************************************/

SDLTextButton::SDLTextButton(
    SDLCamera* camera,
    const std::string& text,
    int x, int y,
    const SDL_Color& color,
    const SDL_Color& bgcolor
) : SDLButton(x,y), camera_(camera), text_(text) {
    // build texture with text
    SDLText stext(text, FontLib::fontFamily(), FontLib::fontSize(), color, bgcolor);
    stext.set_position(x,y);
    SDL_Texture* texture = stext.texture(camera->main_renderer()) ;
    setTexture(texture);
    stext.releaseTexture();
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

SDLItemToolButton::SDLItemToolButton(
    const BasicItem& item, SDLTool* tool, std::string icon_name, int x, int y
) : SDLToolButton(tool, icon_name, x, y), item_(item) {
    setTooltipPosition(TooltipPosition::OVER);
}

/*!
 * \return the number of items available in the Command Center
 */
const std::string& SDLItemToolButton::text() const {
    CommandCenter* cc = CommandCenter::cur_command_center;
    SDLItemToolButton* non_const_this = const_cast<SDLItemToolButton*>(this);
    if( cc == nullptr ) {
        non_const_this->setText(std::string());
    } else {
        // get number of items in Command Center
        int nb = cc->countedItems(item_);
        if( nb == 0 ) {
            non_const_this->setText("-");
        } else {
            non_const_this->setText(Utility::itos(nb));
        }
    }
    return SDLToolButton::text();
}

/********************************************************************/
