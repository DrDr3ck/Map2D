#include "sdl_camera.h"

#include <iostream>
#include <math.h>
#include <string>

#include "font.h"
#include "path_finding.h"
#include "sdl_button.h"
#include "sdl_tool.h"
#include "sdl_background.h"

/********************************************************************/

void SDL_RenderDrawCircle(SDL_Renderer* renderer, const SDL_Rect& dest) {
    if( renderer == nullptr ) {
       return;
    }
    int rayon = dest.w/2;
    int x_centre = dest.x + rayon;
    int y_centre = dest.y + rayon;
    int x = 0 ;
    int y = rayon ;             // on se place en haut du cercle
    int m = 5 - 4*rayon ;       // initialisation
    while( x <= y ) {
        SDL_RenderDrawPoint( renderer, x+x_centre, y+y_centre ) ;
        SDL_RenderDrawPoint( renderer, y+x_centre, x+y_centre ) ;
        SDL_RenderDrawPoint( renderer, -x+x_centre, y+y_centre ) ;
        SDL_RenderDrawPoint( renderer, -y+x_centre, x+y_centre ) ;
        SDL_RenderDrawPoint( renderer, x+x_centre, -y+y_centre ) ;
        SDL_RenderDrawPoint( renderer, y+x_centre, -x+y_centre ) ;
        SDL_RenderDrawPoint( renderer, -x+x_centre, -y+y_centre ) ;
        SDL_RenderDrawPoint( renderer, -y+x_centre, -x+y_centre ) ;
        if( m > 0 ) {       //choix du point F
            y = y - 1 ;
            m = m - 8*y ;
        }
        x = x + 1 ;
        m = m + 8*x + 4 ;
    }
}

/********************************************************************/

MapView::MapView(SDLCamera* camera, MapData* data, PeopleGroup* group, JobMgr* manager) : data_(data), group_(group), job_manager_(manager),
    map_background_(nullptr), window_background_(nullptr),
    delta_x_(0.), delta_y_(0.), delta_speed_(0.1), translate_x_(0.), translate_y_(0.)
{
    tile_x_ = -1;
    tile_y_ = -1;

    selected_people_ = nullptr;

    camera->setMapView(this);
}

MapView::~MapView() {
    group_ = nullptr;
    data_ = nullptr;
}

/*!
 * Adds a wall in the queue of the job manager at (x,y) if possible.
 */
void MapView::addWall(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new BuildJob(tile_position, "wall_tool", 3500);
    job_manager_->addJob(job);
}

/*!
 * Adds a wall destruction in the queue of the job manager at (x,y) if possible.
 */
void MapView::removeWall(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new DemolishJob(tile_position, "demolish_tool", 2500);
    job_manager_->addJob(job);
}

/*!
 * Adds a floor in the queue of the job manager at (x,y) if possible.
 */
void MapView::addFloor(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new BuildFloorJob(tile_position, "foundation_tool", 1500);
    job_manager_->addJob(job);
}

/*!
 * Adds a floor destruction in the queue of the job manager at (x,y) if possible.
 */
void MapView::removeFloor(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new DemolishFloorJob(tile_position, "demolish_foundation_tool", 1000);
    job_manager_->addJob(job);
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

/*!
 * \return the rectangle of the \p people depending of its position.
 */
SDL_Rect MapView::getPeopleRect(Character* people) const {
    SDL_Rect dest;
    int people_x = people->tilePosition().x;
    int people_y = people->tilePosition().y;
    if( people->validPixelPosition() ) {
        dest.x = people->pixelPosition().x*scaled_tile_size_/64. + scaled_start_x_;
        dest.y = people->pixelPosition().y*scaled_tile_size_/64. + scaled_start_y_;
    } else {
        dest.x = people_x*scaled_tile_size_ + scaled_start_x_;
        dest.y = people_y*scaled_tile_size_ + scaled_start_y_;
    }
    dest.w = scaled_tile_size_;
    dest.h = scaled_tile_size_;
    return dest;
}

/*!
 * \return the rectangle of the given tile position.
 */
SDL_Rect MapView::getTileRect(int tile_x, int tile_y) const {
    SDL_Rect dest;
    dest.x = tile_x*scaled_tile_size_ + scaled_start_x_;
    dest.y = tile_y*scaled_tile_size_ + scaled_start_y_;
    dest.w = scaled_tile_size_;
    dest.h = scaled_tile_size_;
    return dest;
}

void MapView::do_render(Camera* camera, double delay_in_ms) {
    float scale_speed = camera->scale()*camera->scale();
    if( camera->scale() < 1 ) {
        scale_speed = 1./camera->scale() * 1./camera->scale();
    }
    if( delta_x_ != 0 ) {
        translate_x_ += delta_x_ * scale_speed * (delay_in_ms / 1000.);
    }
    if( delta_y_ != 0 ) {
        translate_y_ += delta_y_ * scale_speed * (delay_in_ms / 1000.);
    }

    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    float scale = camera->scale();
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    if( window_background_ == nullptr ) {
        SDL_Surface* bg_surface = Utility::IMGLoad("background.bmp");
        window_background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
        SDL_FreeSurface(bg_surface);
    }
    if( map_background_ == nullptr ) {
        SDL_Surface* bg_surface = Utility::IMGLoad("out.png");
        map_background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
        SDL_FreeSurface(bg_surface);
    }
    sdl_camera->displayTexture(window_background_, NULL);

    int screen_width, screen_height;
    sdl_camera->getSize(screen_width, screen_height);
    scaled_tile_size_ = 64 * scale;
    int map_width = data_->width() * scaled_tile_size_;
    int map_height = data_->height() * scaled_tile_size_;
    int delta_width = (screen_width - map_width)/2;
    int delta_height = (screen_height - map_height)/2;

    scaled_start_x_ = delta_width + translate_x_;
    scaled_start_y_ = delta_height + translate_y_;

    {
        SDL_Rect dest;
        dest.x = scaled_start_x_;
        dest.y = scaled_start_y_;
        dest.w = scaled_tile_size_*data_->width();
        dest.h = scaled_tile_size_*data_->height();
        sdl_camera->displayTexture(map_background_, &dest);
    }

    std::string tile_text;
    ontile_rect_ = {0,0,0,0};
    SDL_Texture* small = nullptr;
    for( int w = 0 ; w < data_->width(); w++ ) {
        for( int h = 0 ; h < data_->height(); h++ ) {
            const Tile& cur = data_->tile(w,h);
            SDL_Rect dest;
            dest.x = w*scaled_tile_size_ + scaled_start_x_;
            dest.y = h*scaled_tile_size_ + scaled_start_y_;
            dest.w = scaled_tile_size_;
            dest.h = scaled_tile_size_;
            small = TileSetLib::instance()->getTextureFromTile(cur, main_renderer);
            if( small != nullptr ) {
                sdl_camera->displayTexture(small, &dest);
            }
            if( tile_x_ == w && tile_y_ == h ) {
                // dessine un carre blanc autour de la tuile 'onTile'
                SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
                SDL_RenderDrawRect(main_renderer, &dest);
                ontile_rect_ = dest;
                tile_text.append(Tile::typeTileToString(cur.type()));
                tile_text.append(": ");
                tile_text.append(Utility::itos(w));
                tile_text.append(" ");
                tile_text.append(Utility::itos(h));
                // debug
                if( cur.type() == Tile::WALL ) {
                    tile_text.append(" - id: ");
                    tile_text.append(Utility::itos(cur.id()));
                }
                // end debug
            }
        }
    }

    for( auto position_object : data_->objects() ) {
        SDL_Rect dest = getTileRect(position_object.x,position_object.y);
        position_object.object->render(sdl_camera, dest);
    }

    for( auto job : job_manager_->jobs() ) {
        int job_x = job->tilePosition().x;
        int job_y = job->tilePosition().y;
        SDL_Rect dest = getTileRect(job_x, job_y);
        job_manager_->render(*job, sdl_camera, dest);
    }

    // display people
    for( Character* people : group_->group() ) {
        SDL_Rect dest = getPeopleRect(people);
        people->render(sdl_camera, dest);
    }

    if( selected_people_ != nullptr ) {
        // display a circle
        SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
        SDL_Rect dest = getPeopleRect(selected_people_);
        SDL_RenderDrawCircle(main_renderer, dest);
        if( tile_x_ == selected_people_->tilePosition().x && tile_y_ == selected_people_->tilePosition().y ) {
            tile_text.append("\nPeople: ");
            tile_text.append(selected_people_->name());
        }
    }

    // Display the tooltip of the tile
    if( !tile_text.empty() ) {
        SDLText text(tile_text, "pixel11", 14, SDLText::black());
        text.set_position(camera->mouse_x()+30,camera->mouse_y()+10);
        sdl_camera->displayText(text, true);
    }
}

void MapView::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    if( sdl_camera == nullptr ) return;
    const SDL_Event& e = sdl_camera->event();
    if( e.type == SDL_KEYDOWN ) {
        if( e.key.keysym.sym == SDLK_LEFT ) {
            delta_x_ = 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_RIGHT ) {
            delta_x_ = -1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_UP ) {
            delta_y_ = 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_DOWN ) {
            delta_y_ = -1 * delta_speed_;
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

    if( e.type == SDL_MOUSEBUTTONDOWN ) {
        if( e.button.button == SDL_BUTTON_RIGHT ) {
            if( selected_people_ != nullptr ) {
                PathFinding path(data_);
                Position end_position = {tile_x_, tile_y_};
                std::vector<Position> positions = path.findPath(selected_people_->tilePosition(), end_position);
                if( positions.size() > 0 ) {
                    ActionBase* action = new MoveAction(selected_people_, positions, 64);
                    selected_people_->setAction( action, "Move to new location");
                }
            }
        } else if( e.button.button == SDL_BUTTON_LEFT ) {
            selected_people_ = nullptr;
            // select a people if any
            for( auto people : group_->group() ) {
                if( people->tilePosition().x == tile_x_ && people->tilePosition().y == tile_y_ ) {
                    selected_people_ = people;
                    break;
                }
            }
        }
    }
}

/*!
 * param[out] tile_x current x tile if valid
 * param[out] tile_y current y tile if valid
 * \return True if current tile is valid, false otherwise
 */
bool MapView::curTile(int& tile_x, int& tile_y) {
    tile_x = tile_x_;
    tile_y = tile_y_;
    return tile_x_ >= 0;
}

/********************************************************************/

SDLText::SDLText(
    const std::string& text,
    const std::string& family,
    int font_size,
    const SDL_Color& color
) : text_(text), family_(family), size_(font_size), texture_(nullptr), color_(color) {
    rect_.w = 0;
    rect_.h = 0;
    rect_.x = 0;
    rect_.y = 0;
}

SDLText::~SDLText() {
    if( texture_ != nullptr ) {
        SDL_DestroyTexture(texture_);
    }
}

void SDLText::set_position(int x, int y) {
    rect_.x = x;
    rect_.y = y;
}

namespace {
    std::vector<std::string> split(const std::string& s, char delimiter) {
       std::vector<std::string> tokens;
       std::string token;
       std::istringstream tokenStream(s);
       while (std::getline(tokenStream, token, delimiter))
       {
          tokens.push_back(token);
       }
       return tokens;
    }
}

SDL_Texture* SDLText::texture(SDL_Renderer* renderer) {
    SDL_Rect final_rect = {0,0,0,0};
    if( texture_ == nullptr ) {
        // split text according to \n
        std::vector<std::string> texts = split(text_, '\n');
        std::vector<SDL_Surface*> surfaces;
        for( auto text : texts ) {
            TTF_Font* font = FontLib::instance()->getFont(family_, size_);
            TTF_SizeText(font,text.c_str(),&rect_.w,&rect_.h);
            SDL_Surface* texte = TTF_RenderText_Solid(font, text.c_str(), color_);
            surfaces.push_back(texte);
            final_rect.w = std::max<int>(final_rect.w, rect_.w);
            if( final_rect.h > 0 ) {
                final_rect.h += 2;
            }
            final_rect.h += rect_.h;
        }
        SDL_Surface* final_dst = SDL_CreateRGBSurface(0, final_rect.w, final_rect.h, 32, 0, 0, 0, 0);
        SDL_FillRect(final_dst, NULL, SDL_MapRGB(final_dst->format, 255, 255, 255));
        // create SDL_Surface of size: final_rect
        SDL_Rect cur_rect = {0,0,0,0};
        for( auto surface : surfaces ) {
            cur_rect.w = surface->w;
            cur_rect.h = surface->h;
            SDL_Rect source_rect = {0,0,surface->w,surface->h};
            SDL_BlitSurface(
                surface,
                &source_rect,
                final_dst,
                &cur_rect
            );
            cur_rect.y = cur_rect.y + 2 + cur_rect.h;
            SDL_FreeSurface(surface);
        }
        texture_ = SDL_CreateTextureFromSurface(renderer, final_dst);
        rect_.w = final_rect.w;
        rect_.h = final_rect.h;
    }
    return texture_;
}

/********************************************************************/

SDLCamera::SDLCamera() : Camera(), window_(nullptr), main_renderer_(nullptr), tool_(nullptr), map_view_(nullptr) {
    if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
        window_ = SDL_CreateWindow("Tile Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
        main_renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        TTF_Init();
    }
    manager_ = new SDLButtonManager();

    // Add Wall menu
    int max_column = 4;
    MenuButton* wall_menu = new MenuButton(max_column, 10, 75);

    SDLBuildTool* wall_tool = new SDLBuildTool(this, "wall_tool.png", WALLTOOL);
    SDLButton* wall_button_tool = new SDLToolButton(wall_tool, "wall_tool.png", 0, 0);
    manager_->addButton(wall_button_tool);
    wall_menu->addButton(wall_button_tool);

    SDLUnbuildTool* demolish_tool = new SDLUnbuildTool(this, "demolish_tool.png", WALLTOOL);
    SDLButton* demolish_button_tool = new SDLToolButton(demolish_tool, "demolish_tool.png", 0, 0);
    manager_->addButton(demolish_button_tool);
    wall_menu->addButton(demolish_button_tool);

    manager_->addButton( new SDLButtonMenu(wall_menu, "wall.png", 10,10) );
    manager_->addMenuButton( wall_menu );

    // Add Floor menu
    MenuButton* floor_menu = new MenuButton(max_column, 70, 75);

    SDLBuildTool* foundation_tool = new SDLBuildTool(this, "foundation_tool.png", FLOORTOOL);
    SDLButton* foundation_button_tool = new SDLToolButton(foundation_tool, "foundation_tool.png", 0, 0);
    manager_->addButton(foundation_button_tool);
    floor_menu->addButton(foundation_button_tool);

    SDLUnbuildTool* demolish_foundation_tool = new SDLUnbuildTool(this, "foundation_tool.png", FLOORTOOL); // demolish_foundation_tool.png
    SDLButton* demolish_foundation_button_tool = new SDLToolButton(demolish_foundation_tool, "foundation_tool.png", 0, 0); // demolish_foundation_tool.png
    manager_->addButton(demolish_foundation_button_tool);
    floor_menu->addButton(demolish_foundation_button_tool);

    manager_->addButton( new SDLButtonMenu(floor_menu, "floor.png", 70,10) );
    manager_->addMenuButton( floor_menu );

    // Add Object menu
    MenuButton* object_menu = new MenuButton(max_column, 130, 75);

    SDLBuildTool* chest_tool = new SDLBuildTool(this, "chest.png", OBJECTTOOL);
    SDLButton* chest_button_tool = new SDLToolButton(chest_tool, "chest.png", 0, 0);
    manager_->addButton(chest_button_tool);
    object_menu->addButton(chest_button_tool);

    manager_->addButton( new SDLButtonMenu(object_menu, "object.png", object_menu->x(), 10) );
    manager_->addMenuButton( object_menu );

    // Add Quit Button
    manager_->addButton( new SDLQuitButton(this, 750,10) );
}

SDLCamera::~SDLCamera() {
    // Quit SDL
    if( main_renderer_ != nullptr ) {
        SDL_DestroyRenderer(main_renderer_);
    }
    if( window_ != nullptr ) {
        SDL_DestroyWindow(window_);
    }
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

void SDLCamera::render(double delay_in_ms) {
    // rendering for all View(s)
    Camera::render(delay_in_ms);
    manager_->do_render(this, delay_in_ms);
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

        SDLText text("Pause (Press SPACE)\nLeft click to select a robot\nRight click to move it", "pixel11", 16, SDLText::black());
        text.set_position(300,70);
        SDL_SetRenderDrawColor( main_renderer_, 250, 250, 250, 255 );
        text.texture(main_renderer_); // need to create texture in order to get correct text dimension
        SDL_RenderFillRect( main_renderer_, &text.rect() );
        displayText(text);
    }

    // debug: display mouse position
    std::string mouse_position;
    mouse_position.append(Utility::itos(mouse_x()));
    mouse_position.append(" ");
    mouse_position.append(Utility::itos(mouse_y()));
    SDLText text(mouse_position, "pixel11", 14, SDLText::red());
    text.set_position(10,580);
    displayText(text);
    // end debug

    // render tool
    if( tool_ != nullptr ) {
        SDL_Texture* texture = tool_->getTexture(main_renderer_);
        SDL_Rect rect = tool_->rect();
        rect.w *= scale();
        rect.h *= scale();
        SDL_Rect tilerect = map_view_->onTileRect();
        rect.x = tilerect.x;
        rect.y = tilerect.y;
        rect.y = rect.y + 64*scale() - rect.h;
        if( tilerect.w != 0 ) {
            displayTexture(texture, &rect);
        }
    }

    SDL_RenderPresent(main_renderer_);
}

void SDLCamera::displayTexture(SDL_Texture* texture, const SDL_Rect* rect) {
    int check = SDL_RenderCopy(main_renderer_, texture, NULL, rect);
    if( check != 0 ) {
        std::cout << "Check = " << check << "  " << SDL_GetError() << std::endl;
    }
}

void SDLCamera::displayText(SDLText& text, bool background) {
    SDL_Texture* texture = text.texture(main_renderer_);
    if( background ) {
        SDL_Rect rect = text.rect();
        rect.x = rect.x - 5;
        rect.w = rect.w + 10;
        rect.y = rect.y - 2;
        rect.h = rect.h + 4;
        SDL_SetRenderDrawColor( main_renderer_, 250, 250, 250, 255 );
        SDL_RenderFillRect( main_renderer_, &rect );
        SDL_SetRenderDrawColor( main_renderer_, 50, 50, 50, 255 );
        SDL_RenderDrawRect(main_renderer_, &rect);
    }
    displayTexture(texture, &text.rect());
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
            } else if( event_.key.keysym.sym == SDLK_b ) {
                BackGroundGenerator generator(10,6);
                generator.execute("out.png");
            }
            break;
        case SDL_MOUSEMOTION:
            onMouseMove(event_.motion.x, event_.motion.y);
            break;
        case SDL_MOUSEWHEEL:
            onMouseWheelScroll(event_.wheel.x, event_.wheel.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if( event_.button.button == SDL_BUTTON_RIGHT ) {
                setTool(nullptr);
            }
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

void SDLCamera::setTool(SDLTool* tool) {
    tool_ = tool;
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
