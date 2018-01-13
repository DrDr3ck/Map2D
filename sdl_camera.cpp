#include "sdl_camera.h"

#include <iostream>

/********************************************************************/

MapView::MapView(MapData* data) : data_(data), background_(nullptr),
    scale_(1.), delta_x_(0.), delta_y_(0.), delta_speed_(0.1)
{
}

void MapView::do_render(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    if( background_ == nullptr ) {
        SDL_Surface* bg_surface = SDL_LoadBMP("background.bmp");
        background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
        SDL_FreeSurface(bg_surface);
    }
    sdl_camera->displayTexture(background_, NULL);

    int screen_width, screen_height;
    sdl_camera->getSize(screen_width, screen_height);
    int tile_size = 64 * scale_;
    int map_width = data_->width() * tile_size;
    int map_height = data_->height() * tile_size;
    int delta_width = (screen_width - map_width)/2;
    int delta_height = (screen_height - map_height)/2;

    SDL_Texture* small = nullptr;
    for( int w = 0 ; w < data_->width(); w++ ) {
        for( int h = 0 ; h < data_->height(); h++ ) {
            const Tile& cur = data_->tile(w,h);
            small = TileSet::instance()->getTextureFromTile(cur, main_renderer);
            SDL_Rect dest;
            dest.x = w*tile_size + delta_width + delta_x_;
            dest.y = h*tile_size + delta_height + delta_y_;
            dest.w = tile_size;
            dest.h = tile_size;
            sdl_camera->displayTexture(small, &dest);
        }
    }
}

void MapView::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    if( sdl_camera == nullptr ) return;
    const SDL_Event& e = sdl_camera->event();
    if( e.type == SDL_KEYDOWN ) {
        if( e.key.keysym.sym == SDLK_LEFT ) {
            delta_x_ -= 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_RIGHT ) {
            delta_x_ += 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_UP ) {
            delta_y_ -= 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_DOWN ) {
            delta_y_ += 1 * delta_speed_;
        }
    }
}

/********************************************************************/

SDLCamera::SDLCamera() : Camera(), window_(nullptr), main_renderer_(nullptr), font_(nullptr) {
    if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
        window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
        main_renderer_ = SDL_CreateRenderer(window_, -1, 0);
        TTF_Init();
        font_ = TTF_OpenFont("pixel11.ttf", 24);
    }
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
        SDL_RenderCopy(main_renderer_, text, NULL, &position);
        SDL_DestroyTexture(text);
        SDL_FreeSurface(texte);
    }
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

    // handle event for all View(s)
    Camera::handleEvent();
}

void SDLCamera::do_quit() const {
    if( !valid() ) {
        std::cout << "Camera not valid: " << SDL_GetError() << std::endl;
    }
    SDL_Quit();
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
