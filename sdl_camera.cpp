#include "sdl_camera.h"

#include <iostream>
#include <math.h>

/********************************************************************/

MapView::MapView(MapData* data) : data_(data), background_(nullptr),
    delta_x_(0.), delta_y_(0.), delta_speed_(0.1), translate_x_(0.), translate_y_(0.)
{
}

/*!
 * \return true and the tile position according to the mouse position
 * or false if unreached
 */
bool MapView::onTile(int mouse_x, int mouse_y) {
    tile_x_ = -1;
    tile_y_ = -1;
    if( mouse_x < scaled_start_x_ ) return false;
    if( mouse_y < scaled_start_y_ ) return false;
    int map_width = data_->width() * scaled_tile_size_;
    int map_height = data_->height() * scaled_tile_size_;
    if( mouse_x > scaled_start_x_ + map_width ) return false;
    if( mouse_y > scaled_start_y_ + map_height ) return false;
    tile_x_ = floor((mouse_x - scaled_start_x_) / scaled_tile_size_);
    tile_y_ = floor((mouse_y - scaled_start_y_) / scaled_tile_size_);
    return true;
}

void MapView::do_render(Camera* camera) {
    // should it be done here ?
    if( delta_x_ != 0 ) {
        translate_x_ += delta_x_ * camera->scale()*1;
    }
    if( delta_y_ != 0 ) {
        translate_y_ += delta_y_ * camera->scale()*1;
    }

    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    float scale = camera->scale();
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    if( background_ == nullptr ) {
        SDL_Surface* bg_surface = SDL_LoadBMP("background.bmp");
        background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
        SDL_FreeSurface(bg_surface);
    }
    sdl_camera->displayTexture(background_, NULL);

    int screen_width, screen_height;
    sdl_camera->getSize(screen_width, screen_height);
    scaled_tile_size_ = 64 * scale;
    int map_width = data_->width() * scaled_tile_size_;
    int map_height = data_->height() * scaled_tile_size_;
    int delta_width = (screen_width - map_width)/2;
    int delta_height = (screen_height - map_height)/2;

    scaled_start_x_ = delta_width + translate_x_;
    scaled_start_y_ = delta_height + translate_y_;

    SDL_Texture* small = nullptr;
    for( int w = 0 ; w < data_->width(); w++ ) {
        for( int h = 0 ; h < data_->height(); h++ ) {
            const Tile& cur = data_->tile(w,h);
            small = TileSet::instance()->getTextureFromTile(cur, main_renderer);
            SDL_Rect dest;
            dest.x = w*scaled_tile_size_ + scaled_start_x_;
            dest.y = h*scaled_tile_size_ + scaled_start_y_;
            dest.w = scaled_tile_size_;
            dest.h = scaled_tile_size_;
            sdl_camera->displayTexture(small, &dest);
            if( tile_x_ == w && tile_y_ == h ) {
                SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
                SDL_RenderDrawRect(main_renderer, &dest);
            }
        }
    }
}

void MapView::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    if( sdl_camera == nullptr ) return;
    const SDL_Event& e = sdl_camera->event();
    if( e.type == SDL_KEYDOWN ) {
        if( e.key.keysym.sym == SDLK_LEFT ) {
            delta_x_ = -1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_RIGHT ) {
            delta_x_ = 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_UP ) {
            delta_y_ = -1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_DOWN ) {
            delta_y_ = 1 * delta_speed_;
        }
    }
    if( e.type == SDL_KEYUP ) {
        if( e.key.keysym.sym == SDLK_LEFT ) {
            delta_x_ = 0;
        } else if( e.key.keysym.sym == SDLK_RIGHT ) {
            delta_x_ = 0;
        } else if( e.key.keysym.sym == SDLK_UP ) {
            delta_y_ = 0;
        } else if( e.key.keysym.sym == SDLK_DOWN ) {
            delta_y_ = 0;
        }
    }
}

/********************************************************************/

SDLCamera::SDLCamera() : Camera(), window_(nullptr), main_renderer_(nullptr), font_(nullptr), tool_(nullptr), map_view_(nullptr) {
    if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
        window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
        main_renderer_ = SDL_CreateRenderer(window_, -1, 0);
        TTF_Init();
        font_ = TTF_OpenFont("pixel11.ttf", 24);
    }
    manager_ = new SDLButtonManager();
    manager_->addButton( new SDLQuitButton(this, 10,10) );
    manager_->addButton( new SDLButton("wall.bmp", 10,530) );
}

SDLCamera::~SDLCamera() {
    // Quit SDL
    if( main_renderer_ != nullptr ) {
        SDL_DestroyRenderer(main_renderer_);
    }
    if( window_ != nullptr ) {
        SDL_DestroyWindow(window_);
    }
    TTF_CloseFont(font_);
    TTF_Quit();
    delete manager_;
    do_quit();
}

/*!
 * \return true if this camera is valid
 */
bool SDLCamera::valid() const {
    return window_ != nullptr;
}

void SDLCamera::render() {
    // rendering for all View(s)
    Camera::render();
    manager_->do_render(this);
    if( pause_ ) {
        SDL_SetRenderDrawColor( main_renderer_, 250, 250, 250, 255 );
        SDL_Rect r;
        r.x = 360;
        r.y = 240;
        r.w = 70;
        r.h = 70;
        SDL_RenderFillRect( main_renderer_, &r );
        SDL_SetRenderDrawColor( main_renderer_, 222, 50, 50, 255 );
        r.x = 370;
        r.y = 250;
        r.w = 20;
        r.h = 50;
        SDL_RenderFillRect( main_renderer_, &r );
        r.x = 400;
        SDL_RenderFillRect( main_renderer_, &r );

        // todo: create utility to display text
        SDL_Color black = {0, 0, 0, 255};
        std::string pause("Pause (Press SPACE)");
        int font_size = 20;
        SDL_Surface* texte = TTF_RenderText_Solid(font_, pause.c_str(), black);
        SDL_Texture* text = SDL_CreateTextureFromSurface(main_renderer_, texte);
        SDL_Rect position;
        position.w = pause.size() * font_size/2;
        position.h = font_size;
        position.x = 300;
        position.y = 50;
        SDL_SetRenderDrawColor( main_renderer_, 250, 250, 250, 255 );
        SDL_RenderFillRect( main_renderer_, &position );
        SDL_RenderCopy(main_renderer_, text, NULL, &position);
        SDL_DestroyTexture(text);
        SDL_FreeSurface(texte);
    }
    // ask map_view_ to get the text for the selected tile
    // display text in main_renderer
    SDL_RenderPresent(main_renderer_);
}

void SDLCamera::displayTexture(SDL_Texture* texture, const SDL_Rect* rect) {
    SDL_RenderCopy(main_renderer_, texture, NULL, rect);
}

/*!
 * Gets screen size in pixel.
 */
void SDLCamera::getSize(int& screen_width, int& screen_height) {
    SDL_GetRendererOutputSize(main_renderer_, &screen_width, &screen_height);
}

void SDLCamera::handleEvent() {
    SDL_GetMouseState(&mouse_x_, &mouse_y_);

    if( SDL_PollEvent(&event_) == 0 ) {
        return;
    }
    switch( event_.type ) {
        case SDL_QUIT:
            quit_ = true;
            return;
        case SDL_KEYDOWN:
            if( event_.key.keysym.sym == SDLK_SPACE ) {
                pause_ = !pause_;
            } else if( event_.key.keysym.sym == SDLK_ESCAPE ) {
                quit_ = true;
            }
            break;
        case SDL_MOUSEMOTION:
            onMouseMove(event_.motion.x, event_.motion.y);
            break;
        case SDL_MOUSEWHEEL:
            onMouseWheelScroll(event_.wheel.x, event_.wheel.y);
            break;
        default:
            break;
    }

    if( tool_ != nullptr ) {
        tool_->handleEvent();
    }

    // handle event for all View(s)
    Camera::handleEvent();
    manager_->handleEvent(this);
}

void SDLCamera::onMouseMove(int mouse_x, int mouse_y) {
    Camera::onMouseMove(mouse_x,mouse_y);
    // check on which 'tile' we are
    map_view_->onTile(mouse_x,mouse_y);
}

void SDLCamera::onMouseWheelScroll(int wheel_x, int wheel_y) {
    Camera::onMouseWheelScroll(wheel_x,wheel_y);
}

void SDLCamera::setMapView(MapView* view) {
    map_view_ = view;
    addView(map_view_);
}

void SDLCamera::do_quit() const {
    if( !valid() ) {
        std::cout << "Camera not valid: " << SDL_GetError() << std::endl;
    }
    SDL_Quit();
}

/********************************************************************/

void SDLButtonManager::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    const SDL_Event& e = sdl_camera->event();
    bool button_pressed = false;
    if( e.type == SDL_MOUSEBUTTONDOWN ) {
        button_pressed = true;
    }
    for( auto button : buttons_ ) {
        // get mouse position
        if( button->mouseOverButton(camera->mouse_x(), camera->mouse_y()) ) {
            button->hasFocus(true);
            if( button_pressed ) {
                button->activate();
            }
        } else {
            button->hasFocus(false);
        }
    }
}

void SDLButtonManager::do_render(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    // iterate over all visible buttons, get texture and display them
    for( auto b : buttons_ ) {
        SDLButton* button = dynamic_cast<SDLButton*>(b);
        if( !button->isVisible() ) {
            continue;
        }
        SDL_Texture* texture = button->getTexture(main_renderer);
        sdl_camera->displayTexture(texture, &button->rect());
        if( button->hasFocus() ) {
            SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
            SDL_RenderDrawRect(main_renderer, &button->rect());
        }
    }
}

/********************************************************************/

SDLButton::SDLButton(std::string name, int x, int y) : Button(x,y) {
    surface_ = SDL_LoadBMP(name.c_str());
    int w = surface_->w;
    int h = surface_->h;
    setSize(w,h);
    rect_ = {x,y,w,h};
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
    }
    return texture_;
}

void SDLButton::activate() {
    Button::activate();
}

void SDLButton::deactivate() {
    Button::deactivate();
}

/********************************************************************/

void SDLQuitButton::activate() {
    camera_->set_quit();
}

/********************************************************************/

void SDLTool::handleEvent() {
}

void SDLTool::mousePress() {
}

void SDLTool::mouseMotion() {
}

void SDLTool::mouseRelease() {
}

/********************************************************************/
