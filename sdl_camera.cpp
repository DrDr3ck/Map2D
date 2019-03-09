#include "sdl_camera.h"

#include <iostream>
#include <math.h>
#include <string>

#include "path_finding.h"
#include "sdl_button.h"
#include "sdl_tool.h"
#include "sdl_background.h"
#include "texture_mgr.h"
#include "translator.h"
#include "action.h"
#include "session.h"
#include "dialog.h"

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
// the static approach: https://www.youtube.com/watch?v=HgMNjIZnQhM&t=2s
MapView* MapView::cur_map = nullptr;

MapView::MapView(SDLCamera* camera, MapData* data, PeopleGroup* group, JobMgr* manager) : data_(data), group_(group), job_manager_(manager),
    map_background_(nullptr), window_background_(nullptr), camera_(camera),
    delta_x_(0.), delta_y_(0.), delta_speed_(0.1), translate_x_(0.), translate_y_(0.)
{
    tile_x_ = -1;
    tile_y_ = -1;

    selected_people_ = nullptr;

    camera->setMapView(this);
    camera->getSize(center_x_, center_y_);
    center_x_ /= 2;
    center_y_ /= 2;

    cur_map = this;
}

MapView::~MapView() {
    group_ = nullptr;
    data_ = nullptr;
    camera_ = nullptr;
}

/*!
 * Adds a wall in the queue of the job manager at (x,y) if possible.
 */
void MapView::addWallJob(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new BuildJob(tile_position, "buttons/wall_tool", 3500);
    job_manager_->addJob(job);
}

/*!
 * Adds a wall destruction in the queue of the job manager at (x,y) if possible.
 */
void MapView::removeWallJob(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new DemolishJob(tile_position, "buttons/demolish_tool", 2500);
    job_manager_->addJob(job);
}

void MapView::extractItemJob(int x, int y, int nb) {
    Position tile_position = {x,y};
    Tile cur_tile = data_->tile(x,y);
    Job* job = nullptr;
    switch(cur_tile.background_type()) {
        case Tile::ROCK:
        case Tile::COPPER:
        case Tile::IRON:
        case Tile::COAL:
            job = new ExtractJob(tile_position, "buttons/extract_tool", 5000, nb);
            break;
        case Tile::SAND:
            job = new ExtractJob(tile_position, "buttons/dig_tool", 5000, nb);
            break;
        default:
            break;
    }

    if( job != nullptr ) {
        job_manager_->addJob(job);
    }
}

void MapView::cleanItemJob(int x, int y) {
    Position tile_position = {x,y};
    Tile cur_tile = data_->tile(x,y);
    // first, check if a chest is available
    Object* object = data()->getNearestChest(tile_position);
    if( object == nullptr ) {
        Logger::warning() << "Cannot do cleaning without a chest" << Logger::endl;
        return;
    }
    Job* job = new CleanJob(tile_position, "buttons/clean_tool", 0);
    job_manager_->addJob(job);
}

/*!
 * Adds a floor in the queue of the job manager at (x,y) if possible.
 */
void MapView::addFloorJob(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new BuildFloorJob(tile_position, "buttons/foundation_tool", 1500);
    job_manager_->addJob(job);
}

/*!
 * Adds a floor destruction in the queue of the job manager at (x,y) if possible.
 */
void MapView::removeFloorJob(int x, int y) {
    Position tile_position = {x,y};
    Job* job = new DemolishFloorJob(tile_position, "buttons/demolish_foundation_tool", 1000);
    job_manager_->addJob(job);
}

/*!
 * Adds an object construction in the queue of the job manager at (x,y) if possible.
 */
void MapView::addObjectJob(const std::string& object_name, int x, int y) {
    Position tile_position = {x,y};
    std::string icon_name = "objects/"+object_name;
    Job* job = new BuildObjectJob(tile_position, icon_name, object_name, 1000);
    job_manager_->addJob(job);
}

/*!
 * Adds an object destruction in the queue of the job manager at (x,y) if possible.
 */
void MapView::removeObjectJob(int x, int y) {
    Position tile_position = {x,y};
    // TODO check that an object is present on this tile !!
    Job* job = new UnbuildObjectJob(tile_position, "buttons/uninstall_tool", 1000);
    job_manager_->addJob(job);
}

/*!
 * \return the object that is at tile position \p x, \p y or nullptr if no object.
 */
Object* MapView::getObject(int x,int y) const {
    for( auto object : data()->objects() ) {
        if( object->tilePosition().x == x && object->tilePosition().y == y ) {
            return object;
        }
    }
    return nullptr;
}

/*!
 * \return true and the tile position according to the mouse position
 * or false if unreached
 */
Position MapView::onTile(int mouse_x, int mouse_y) const {
    Position pos;
    pos.x = -1;
    pos.y = -1;
    if( mouse_x < scaled_start_x_ ) return pos;
    if( mouse_y < scaled_start_y_ ) return pos;
    int map_width = data_->width() * scaled_tile_size_;
    int map_height = data_->height() * scaled_tile_size_;
    if( mouse_x > scaled_start_x_ + map_width ) return pos;
    if( mouse_y > scaled_start_y_ + map_height ) return pos;
    pos.x = floor((mouse_x - scaled_start_x_) / scaled_tile_size_);
    pos.y = floor((mouse_y - scaled_start_y_) / scaled_tile_size_);
    return pos;
}

void MapView::setTile(int tile_x, int tile_y) {
    tile_x_ = tile_x;
    tile_y_ = tile_y;
}

/*!
 * \return the rectangle of the \p people depending of its position.
 */
SDL_Rect MapView::getPeopleRect(Character* people) const {
    SDL_Rect dest;
    int people_x = people->tilePosition().x;
    int people_y = people->tilePosition().y;
    if( people->validPixelPosition() ) {
        dest.x = people->pixelPosition().x*scaled_tile_size_/float(Utility::tileSize) + scaled_start_x_;
        dest.y = people->pixelPosition().y*scaled_tile_size_/float(Utility::tileSize) + scaled_start_y_;
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

// render objects
void MapView::renderObjects(SDLCamera* sdl_camera, std::string& tile_text) {
    for( auto object : data_->objects() ) {
        Position position_object = object->tilePosition();
        SDL_Rect dest = getTileRect(position_object.x,position_object.y);
        object->render(sdl_camera, dest);
        if( tile_x_ == position_object.x && tile_y_ == position_object.y ) {
            // add object name on tile tooltip
            tile_text.append("\n");
            tile_text.append(object->tooltip());
        }
    }
}

// render jobs
void MapView::renderJobs(SDLCamera* sdl_camera) {
    // display jobs (in semi transparency)
    for( auto job : job_manager_->jobs() ) {
        int job_x = job->tilePosition().x;
        int job_y = job->tilePosition().y;
        SDL_Rect dest = getTileRect(job_x, job_y);
        job_manager_->render(*job, sdl_camera, dest);
    }
}

void MapView::renderGroup(SDLCamera* sdl_camera) {
    // display people
    for( Character* people : group_->group() ) {
        SDL_Rect dest = getPeopleRect(people);
        people->render(sdl_camera, dest);
    }
}

void MapView::do_render(Camera* camera, double delay_in_ms) {
    float scale_speed = camera->speed();
    if( delta_x_ != 0 ) {
        translate_x_ += delta_x_ * scale_speed * delay_in_ms;
    }
    if( delta_y_ != 0 ) {
        translate_y_ += delta_y_ * scale_speed * delay_in_ms;
    }

    // compute background (one time only)
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    float scale = camera->scale();
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    if( window_background_ == nullptr ) {
        SDL_Surface* bg_surface = Utility::IMGLoad("images/background.png");
        window_background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
        SDL_FreeSurface(bg_surface);
    }
    if( map_background_ == nullptr ) {
        SDL_Surface* bg_surface = Utility::IMGLoad(data()->getMapImageName());
        map_background_ = SDL_CreateTextureFromSurface(main_renderer, bg_surface);
        SDL_FreeSurface(bg_surface);
    }
    sdl_camera->displayTexture(window_background_, NULL);

    int screen_width, screen_height;
    sdl_camera->getSize(screen_width, screen_height);
    scaled_tile_size_ = Utility::tileSize * scale;
    int map_width = data_->width() * scaled_tile_size_;
    int map_height = data_->height() * scaled_tile_size_;
    int delta_width = (screen_width - map_width)/2;
    int delta_height = (screen_height - map_height)/2;

    scaled_start_x_ = delta_width + translate_x_;
    scaled_start_y_ = delta_height + translate_y_;

    // display background with scale
    if( map_background_ != nullptr ) {
        SDL_Rect dest;
        dest.x = scaled_start_x_;
        dest.y = scaled_start_y_;
        dest.w = scaled_tile_size_*data_->width();
        dest.h = scaled_tile_size_*data_->height();
        sdl_camera->displayTexture(map_background_, &dest);
    }

    // display tiles on Map // wall, floor, door, ...
    std::string tile_text;
    ontile_rect_ = {0,0,0,0};
    SDL_Texture* small = nullptr;
    Position topleft = onTile(0,0);
    if( topleft.x < 0 ) topleft.x = 0;
    if( topleft.y < 0 ) topleft.y = 0;
    Position bottomright = onTile(camera->width()+Utility::tileSize*camera->scale(),camera->height()+Utility::tileSize*camera->scale());
    if( bottomright.x < 0 ) bottomright.x = data_->width();
    if( bottomright.y < 0 ) bottomright.y = data_->height();

    std::vector< SDL_Texture* > items_to_display;
    std::vector< SDL_Rect > items_to_dest;

    for( int w = topleft.x ; w < bottomright.x; w++ ) {
        for( int h = topleft.y ; h < bottomright.y; h++ ) {
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
            // display item
            if( cur.counted_items().size() > 0 ) {
                SDL_Texture* item_texture = cur.counted_items().at(0).texture(); // only display the first element
                SDL_Rect dest;
                int half_scaled_tile_size = scaled_tile_size_/2;
                dest.x = w*scaled_tile_size_ + scaled_start_x_ + half_scaled_tile_size/2;
                dest.y = h*scaled_tile_size_ + scaled_start_y_ + half_scaled_tile_size/2;
                dest.w = half_scaled_tile_size;
                dest.h = half_scaled_tile_size;
                items_to_display.push_back( item_texture );
                items_to_dest.push_back( dest );
            }
            // display white square over the onTile cursor
            if( tile_x_ == w && tile_y_ == h ) {
                SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
                SDL_RenderDrawRect(main_renderer, &dest);
                ontile_rect_ = dest;
                // build tooltip: add background if no wall/floor
                if( Tile::isWall(cur) ) {
                    tile_text.append(tr("Wall"));
                    // debug
                    //tile_text.append(" - id: ");
                    //tile_text.append(Utility::itos(cur.id()));
                    // end debug
                } else if( Tile::isFloor(cur) ) {
                    tile_text.append(tr("Floor"));
                } else {
                    tile_text.append(Tile::btypeTileToString(cur.background_type()));
                    tile_text.append(" (");
                    tile_text.append(Utility::itos(cur.occurrences()));
                    tile_text.append(")");
                }
                // add position of tile
                tile_text.append(": ");
                tile_text.append(Utility::itos(w));
                tile_text.append(" ");
                tile_text.append(Utility::itos(h));
                // add counted item if any
                if( cur.counted_items().size() > 0 ) {
                    for( auto counted_item : cur.counted_items() ) {
                        tile_text.append("\n");
                        tile_text.append(tr(counted_item.item().name()));
                        tile_text.append(": ");
                        tile_text.append(Utility::itos(counted_item.count()));
                    }
                }
            }
        }
    }

    renderObjects(sdl_camera, tile_text);

    // render items
    for( int i=0; i < int(items_to_display.size()); i++ ) {
        SDL_Texture* item_texture = items_to_display.at(i);
        SDL_Rect dest = items_to_dest.at(i);
        sdl_camera->displayTexture(item_texture, &dest);
    }

    renderJobs(sdl_camera);

    renderGroup(sdl_camera);

    // display a circle around selected people if any
    if( selected_people_ != nullptr ) {
        SDL_SetRenderDrawColor( main_renderer, 250, 250, 250, 255 );
        SDL_Rect dest = getPeopleRect(selected_people_);
        SDL_RenderDrawCircle(main_renderer, dest);
        if( tile_x_ == selected_people_->tilePosition().x && tile_y_ == selected_people_->tilePosition().y ) {
            tile_text.append("\n");
            tile_text.append(tr("People: "));
            tile_text.append(selected_people_->name());
        }
    }

    // Display the tooltip of the tile
    if( !tile_text.empty() ) {
        SDLText text(tile_text, FontLib::fontFamily(), FontLib::fontSize(), SDLText::black());
        text.set_position(camera->mouse_x()+30,camera->mouse_y()+10);
        sdl_camera->displayText(text, true);
    }
}

/*!
 * \return true if the event has been handled.
 */
bool MapView::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    if( sdl_camera == nullptr ) return false;
    const SDL_Event& e = sdl_camera->event();
    switch( e.type ) {
        case SDL_KEYDOWN:
        if( e.key.keysym.sym == SDLK_LEFT ) {
            delta_x_ = 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_RIGHT ) {
            delta_x_ = -1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_UP ) {
            delta_y_ = 1 * delta_speed_;
        } else if( e.key.keysym.sym == SDLK_DOWN ) {
            delta_y_ = -1 * delta_speed_;
        }
        break;
    case SDL_KEYUP:
        if( e.key.keysym.sym == SDLK_LEFT ) {
            delta_x_ = 0;
        } else if( e.key.keysym.sym == SDLK_RIGHT ) {
            delta_x_ = 0;
        } else if( e.key.keysym.sym == SDLK_UP ) {
            delta_y_ = 0;
        } else if( e.key.keysym.sym == SDLK_DOWN ) {
            delta_y_ = 0;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        if( e.button.button == SDL_BUTTON_RIGHT ) {
            if( selected_people_ != nullptr ) {
                PathFinding path(data_);
                Position end_position = {tile_x_, tile_y_};
                std::vector<Position> positions = path.findPath(selected_people_->tilePosition(), end_position);
                if( positions.size() > 0 ) {
                    ActionBase* action = new MoveAction(selected_people_, positions, Utility::tileSize);
                    selected_people_->setAction( action, "Move to new location");
                    return true;
                }
            }
            // canceling a job ?
            // check if a job is on the clicked tile
            Position position = {tile_x_, tile_y_};
            if( job_manager_->findJobAt(position) ) {
                job_manager_->cancelJob(position);
                // need also to check if a robot was not already doing the job
                Character* robot = nullptr;
                for( auto people : group_->group() ) {
                    if( people->tilePosition().x == tile_x_ && people->tilePosition().y == tile_y_ ) {
                        robot = people;
                        break;
                    }
                }
                if( robot->hasAction() ) {
                    robot->cancelAction();
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
            if( e.button.clicks > 1 ) { // double click on a robot
                if( selected_people_ != nullptr ) {
                    RobotDialog* dialog = camera->findRobotDialog(selected_people_);
                    if( dialog == nullptr ) {
                        dialog = new RobotDialog(selected_people_, sdl_camera->mouse_x(),sdl_camera->mouse_y()+10);
                    } else {
                        camera->removeView(dialog);
                    }
                    camera->addView(dialog);
                    return true;
                } else { // an object is selected ?
                    for( auto object : data_->objects() ) {
                        Position position_object = object->tilePosition();
                        if( tile_x_ == position_object.x && tile_y_ == position_object.y ) {
                            ObjectDialog* dialog = camera->findObjectDialog(object);
                            if( dialog == nullptr ) {
                                dialog = ObjectDialog::createDialog(object, sdl_camera->mouse_x(),sdl_camera->mouse_y()+10);
                            } else {
                                camera->removeView(dialog);
                            }
                            camera->addView(dialog);
                            return true;
                        }
                    }
                }
            }
        }
        break;
    default:
        return false;
    }
    return false;
}

/*!
 * param[out] tile_x current x tile if valid
 * param[out] tile_y current y tile if valid
 * \return True if current tile is valid, false otherwise
 */
bool MapView::getCurTile(int& tile_x, int& tile_y) {
    tile_x = tile_x_;
    tile_y = tile_y_;
    return tile_x_ >= 0;
}

Position MapView::getCenterTile() const {
    Position pos = onTile(center_x_, center_y_);
    return pos;
}

void MapView::restoreCenterTile(Position pos) {
    int mid_x = data()->width() / 2;
    mid_x = mid_x - pos.x;
    float translate_x = (mid_x * Utility::tileSize * camera_->scale());
    translate_x_ = translate_x - (Utility::tileSize/2)*camera_->scale();

    int mid_y = data()->height() / 2;
    mid_y = mid_y - pos.y;
    float translate_y = (mid_y * Utility::tileSize * camera_->scale());
    translate_y_ = translate_y - (Utility::tileSize/2)*camera_->scale();
}

PeopleGroup* MapView::group() const {
    return group_;
}

/********************************************************************/

SDLText::SDLText(
    const std::string& text,
    const std::string& family,
    int font_size,
    const SDL_Color& color,
    const SDL_Color& background_color
) : text_(text), family_(family), size_(font_size), texture_(nullptr), color_(color) {
    rect_.w = 0;
    rect_.h = 0;
    rect_.x = 0;
    rect_.y = 0;
    background_color_ = background_color;
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

void SDLText::setBackgroundColor(const SDL_Color& bgcolor) {
    background_color_ = bgcolor;
}

const SDL_Color& SDLText::getBackgroundColor() const {
    return background_color_;
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

void SDLText::releaseTexture() {
    texture_ = nullptr; // enable to not destroy the texture when deleting the SDLText
}

SDL_Texture* SDLText::texture(SDL_Renderer* renderer) {
    SDL_Rect final_rect = {0,0,0,0};
    if( texture_ == nullptr ) {
        // split text according to \n
        std::vector<std::string> texts = split(text_, '\n');
        std::vector<SDL_Surface*> surfaces;
        for( auto text : texts ) {
            if( text.empty() ) continue;
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
        SDL_FillRect(final_dst, NULL, SDL_MapRGBA(final_dst->format, background_color_.r, background_color_.g, background_color_.b, background_color_.a));
        //SDL_SetSurfaceBlendMode(final_dst, SDL_BLENDMODE_BLEND) ;
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
        SDL_SetTextureAlphaMod( texture_, 12 );
        rect_.w = final_rect.w;
        rect_.h = final_rect.h;
        SDL_FreeSurface(final_dst);
    }
    return texture_;
}

/********************************************************************/

namespace {
    void add_object_in_menu(const std::string& object_name, SDLCamera* camera, SDLButtonManager* manager, MenuButton* object_menu) {
        std::string icon_name("objects/");
        icon_name.append( object_name );
        icon_name.append( ".png" );
        SDLBuildObjectTool* tool = new SDLBuildObjectTool(camera, icon_name, object_name);
        SDLButton* button_tool = new SDLItemToolButton(BasicItem(object_name), tool, icon_name, 0, 0);
        manager->addButton(button_tool);
        object_menu->addButton(button_tool);
    }
}

SDLCamera::SDLCamera(
    int width, int height
) : Camera(width, height), window_(nullptr), main_renderer_(nullptr), tool_(nullptr), map_view_(nullptr) {
    window_ = SDL_CreateWindow("Bakhar - Demo 0.1 alpha", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    main_renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    TTF_Init();
    manager_ = new SDLButtonManager();

    // Add foundation menu
    int max_column = 4;
    MenuButton* build_menu = new MenuButton(max_column, 15, 90);

    SDLBuildTool* wall_tool = new SDLBuildTool(this, "buttons/wall_tool.png", WALLTOOL);
    SDLButton* wall_button_tool = new SDLItemToolButton(BasicItem("wall"),wall_tool, "buttons/wall_tool.png", 0, 0);
    manager_->addButton(wall_button_tool);
    build_menu->addButton(wall_button_tool);

    SDLUnbuildTool* demolish_tool = new SDLUnbuildTool(this, "buttons/demolish_tool.png", WALLTOOL);
    SDLButton* demolish_button_tool = new SDLToolButton(demolish_tool, "buttons/demolish_tool.png", 0, 0);
    manager_->addButton(demolish_button_tool);
    build_menu->addButton(demolish_button_tool);

    SDLBuildTool* foundation_tool = new SDLBuildTool(this, "buttons/foundation_tool.png", FLOORTOOL);
    SDLButton* foundation_button_tool = new SDLItemToolButton(BasicItem("floor"),foundation_tool, "buttons/foundation_tool.png", 0, 0);
    manager_->addButton(foundation_button_tool);
    build_menu->addButton(foundation_button_tool);

    SDLUnbuildTool* demolish_foundation_tool = new SDLUnbuildTool(this, "buttons/demolish_foundation_tool.png", FLOORTOOL);
    SDLButton* demolish_foundation_button_tool = new SDLToolButton(demolish_foundation_tool, "buttons/demolish_foundation_tool.png", 0, 0);
    manager_->addButton(demolish_foundation_button_tool);
    build_menu->addButton(demolish_foundation_button_tool);

    SDLButtonMenu* build_button = new SDLButtonMenu(build_menu, "buttons/build.png", build_menu->x(),10);
    build_button->setText( tr("Foundations") );
    manager_->addButton( build_button );
    manager_->addMenuButton( build_menu );

    // Add tools menu
    MenuButton* excavation_menu = new MenuButton(max_column, 85, 90);
    SDLExtractTool* extract_tool = new SDLExtractTool(this, "buttons/extract_tool.png", 1);
    SDLButton* extract_button_tool = new SDLToolButton(extract_tool, "buttons/extract_tool.png", 0, 0);
    manager_->addButton(extract_button_tool);
    excavation_menu->addButton(extract_button_tool);

    extract_tool = new SDLExtractTool(this, "buttons/dig_tool.png", 1);
    extract_button_tool = new SDLToolButton(extract_tool, "buttons/dig_tool.png", 0, 0);
    manager_->addButton(extract_button_tool);
    excavation_menu->addButton(extract_button_tool);

    SDLCleanTool* clean_tool = new SDLCleanTool(this, "buttons/clean_tool.png");
    extract_button_tool = new SDLToolButton(clean_tool, "buttons/clean_tool.png", 0, 0);
    manager_->addButton(extract_button_tool);
    excavation_menu->addButton(extract_button_tool);

    extract_tool = new SDLExtractTool(this, "buttons/extract_tool_10.png", 10);
    extract_button_tool = new SDLToolButton(extract_tool, "buttons/extract_tool_10.png", 0, 0);
    manager_->addButton(extract_button_tool);
    excavation_menu->addButton(extract_button_tool);

    extract_tool = new SDLExtractTool(this, "buttons/dig_tool_10.png", 10);
    extract_button_tool = new SDLToolButton(extract_tool, "buttons/dig_tool_10.png", 0, 0);
    manager_->addButton(extract_button_tool);
    excavation_menu->addButton(extract_button_tool);

    SDLButtonMenu* excavation_button = new SDLButtonMenu(excavation_menu, "buttons/excavation.png", excavation_menu->x(),10);
    excavation_button->setText( tr("Tools") );
    manager_->addButton( excavation_button );
    manager_->addMenuButton( excavation_menu );

    // Add Objects menu
    MenuButton* object_menu = new MenuButton(max_column, 155, 90);

    add_object_in_menu("command_center", this, manager_, object_menu);
    add_object_in_menu("workbench", this, manager_, object_menu);
    add_object_in_menu("chest", this, manager_, object_menu);
    add_object_in_menu("iron_chest", this, manager_, object_menu);
    add_object_in_menu("breaker", this, manager_, object_menu);
    add_object_in_menu("stone_furnace", this, manager_, object_menu);
    add_object_in_menu("assembler", this, manager_, object_menu);

    SDLUnbuildObjectTool* uninstall_tool = new SDLUnbuildObjectTool(this, "buttons/uninstall_tool.png");
    SDLButton* uninstall_button_tool = new SDLToolButton(uninstall_tool, "buttons/uninstall_tool.png", 0, 0);
    manager_->addButton(uninstall_button_tool);
    object_menu->addButton(uninstall_button_tool);

    SDLButtonMenu* object_button = new SDLButtonMenu(object_menu, "buttons/object.png", object_menu->x(),10);
    object_button->setText( tr("Objects") );
    manager_->addButton( object_button );
    manager_->addMenuButton( object_menu );

    // Add Quit Button
    SDLButton* quit_button = new SDLQuitButton(this, Camera::cur_camera->width()-50,10);
    manager_->addButton( quit_button );
    SDLButton* options_button = new SDLButton("buttons/options.png", tr("Options"), Camera::cur_camera->width()-50-20-quit_button->rect().w,10);
    options_button->setTooltipPosition(SDLButton::TooltipPosition::BOTTOM);
    manager_->addButton( options_button );
    manager_->connectButton( options_button, openOptionsDialog);
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
    SDL_Quit();
}

void SDLCamera::init() {
    addView(manager_);
}

void SDLCamera::openOptionsDialog() {
    Logger::debug() << "open options dialog" << Logger::endl; // TODO
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

    // display PAUSE tooltip
    if( pause_ ) {
        // display the 'pause' button
        SDL_SetRenderDrawColor( main_renderer_, 250, 250, 250, 255 );
        SDL_Rect r;
        r.x = Camera::cur_camera->width() / 2 - 40;
        r.y = Camera::cur_camera->height() / 2 - 60;
        r.w = 70;
        r.h = 70;
        SDL_RenderFillRect( main_renderer_, &r );
        SDL_SetRenderDrawColor( main_renderer_, 222, 50, 50, 255 );
        r.x = Camera::cur_camera->width() / 2 - 30;
        r.y = Camera::cur_camera->height() / 2 - 50;
        r.w = 20;
        r.h = 50;
        SDL_RenderFillRect( main_renderer_, &r );
        r.x = Camera::cur_camera->width() / 2;
        SDL_RenderFillRect( main_renderer_, &r );

        std::vector<std::string> option_list;
        option_list.push_back( tr("Pause (Press SPACE)") );
        option_list.push_back( tr("Press c to center a robot") );
        option_list.push_back( tr("Left click to select a robot") );
        option_list.push_back( tr("Right click to move it") );
        option_list.push_back( tr("Double click on object to popup info") );
        std::string options;
        for( auto opt : option_list ) {
            if( options.length() != 0 ) {
                options.append("\n");
            }
            options.append(opt);
        }
        SDLText text(options, FontLib::fontFamily(), FontLib::fontSize(), SDLText::black());
        text.set_position(0,0);
        SDL_SetRenderDrawColor( main_renderer_, 250, 250, 250, 255 );
        text.texture(main_renderer_); // need to create texture in order to get correct text dimension
        text.set_position(Camera::cur_camera->width()/2 - text.rect().w / 2, 120 - text.rect().h / 2);
        SDL_RenderFillRect( main_renderer_, &text.rect() );
        displayText(text);
    }

    // F3: display mouse position
    if( Session::instance()->getBoolean("*display_F3", true) ) {
        std::string mouse_position;
        mouse_position.append(Utility::itos(mouse_x()));
        mouse_position.append(" ");
        mouse_position.append(Utility::itos(mouse_y()));
        SDLText text(mouse_position, FontLib::fontFamily(), FontLib::fontSize(), SDLText::red());
        text.set_position(10,Camera::cur_camera->height()-30);
        displayText(text);
    }

    // render tool
    if( tool_ != nullptr ) {
        SDL_Texture* texture = tool_->getTexture(main_renderer_);
        SDL_Rect rect = tool_->rect();
        rect.w *= scale();
        rect.h *= scale();
        SDL_Rect tilerect = map_view_->onTileRect();
        rect.x = tilerect.x;
        rect.y = tilerect.y;
        rect.y = rect.y + Utility::tileSize*scale() - rect.h;
        if( tilerect.w != 0 ) {
            displayTexture(texture, &rect);
        }
    }

    // render Logger
    const std::vector<LoggerString>& strings = LoggerMgr::instance()->terminal();
    int max_terminal_strings = 10;
    int text_offset = 0;
    for( unsigned int i=0; i < strings.size(); i++ ) {
        const LoggerString& log = strings[i];
        if( log.expired() ) {
            LoggerMgr::instance()->moveInJournal(i);
            i--;
        } else if( max_terminal_strings > 0 ) {
            // display logger string
            SDLText text(log.full_string(), FontLib::fontFamily(), FontLib::fontSize(), (log.type() == "Info") ? SDLText::green() : ((log.type() == "Error") ? SDLText::red() : SDLText::black()));
            text.set_position(30,Camera::cur_camera->height()-50-text_offset);
            this->displayText(text, true);
            max_terminal_strings--;
            text_offset += text.rect().h + 3;
        }
    }

    SDL_RenderPresent(main_renderer_);
}

void SDLCamera::displayTexture(SDL_Texture* texture, const SDL_Rect* rect, const SDL_Rect* source) {
    int check = SDL_RenderCopy(main_renderer_, texture, source, rect);
    if( check != 0 ) {
        Logger::error() << "Error in displayTexture: " << SDL_GetError() << Logger::endl;
    }
}
void SDLCamera::displayButton(SDLButton* button, int offset_x, int offset_y) {
    if( button == nullptr ) return;
    if( !button->isVisible() ) return;
    SDL_Texture* button_texture = button->getTexture(main_renderer());
    SDL_Rect rect = button->rect();
    rect.x = rect.x+offset_x;
    rect.y = rect.y+offset_y;
    displayTexture(button_texture, &rect);
    if( !button->text().empty()) {
        SDLText button_text( button->text(), FontLib::fontFamily(), FontLib::fontButtonSize());
        if( button->tooltipPosition() == SDLButton::TooltipPosition::BOTTOM ) {
            button_text.texture(main_renderer());
            SDL_Rect text_rect = button_text.rect();
            button_text.set_position(button->rect().x +button->rect().w/2 - text_rect.w/2 + offset_x, button->rect().y+button->rect().h + offset_y);
        } else if( button->tooltipPosition() == SDLButton::TooltipPosition::OVER ) {
            button_text.texture(main_renderer());
            SDL_Rect text_rect = button_text.rect();
            button_text.set_position(button->rect().x +button->rect().w/2 - text_rect.w/2 + offset_x, button->rect().y + offset_y + 5);
        } else {
            button_text.set_position(button->rect().x+button->rect().w + offset_x, button->rect().y + offset_y);
        }
        displayText(button_text, true, true);
    }
}

void SDLCamera::displayText(SDLText& text, bool background, bool croppable) {
    SDL_Texture* texture = text.texture(main_renderer_);
    if( !croppable ) {
        // if text length is greater than window size, translate the text position
        SDL_Rect rect = text.rect();
        if( rect.x + rect.w > Camera::cur_camera->width() ) {
            int x = rect.x;
            x -= (rect.x + rect.w - Camera::cur_camera->width() );
            text.set_position(x, rect.y);
        }
        if( rect.y + rect.h > Camera::cur_camera->height() ) {
            int y = rect.y;
            y -= (rect.y + rect.h - Camera::cur_camera->height());
            text.set_position(rect.x, y);
        }
    }
    if( background ) {
        // draw a white rectangle with a dark border line
        SDL_Rect rect = text.rect();
        rect.x = rect.x - 5;
        rect.w = rect.w + 10;
        rect.y = rect.y - 2;
        rect.h = rect.h + 4;
        const SDL_Color& bgcolor = text.getBackgroundColor();
        SDL_SetRenderDrawColor( main_renderer_, bgcolor.r, bgcolor.g, bgcolor.b, 255 );
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

/*!
 * Handle keyboard and mouse event
 */
bool SDLCamera::handleEvent() {
    SDL_GetMouseState(&mouse_x_, &mouse_y_);

    if( SDL_PollEvent(&event_) == 0 ) {
        return false;
    }

    bool event_handled = false;
    Dialog* cur_dialog = nullptr;
    for (std::list<View*>::reverse_iterator it = views_.rbegin(); it != views_.rend(); ++it) {
       View* view = *it;
        Dialog* dialog = dynamic_cast<Dialog*>(view);
        if( dialog == nullptr ) continue;
        if( dialog->hasFocus(mouse_x_, mouse_y_) ) {
            event_handled = dialog->handleEvent(this);
            if( dialog->killMe() ) {
                removeView(dialog);
                delete dialog;
                break;
            } else {
                cur_dialog = dialog;
                break;
            }
        }
    }

    if( event_handled ) {
        // put dialog on top !!
        removeView(cur_dialog);
        addView(cur_dialog);
        onMouseMove(-1,-1);
        return true;
    }

    switch( event_.type ) {
        case SDL_QUIT:
            quit_ = true;
            return true;
        case SDL_KEYUP:
            if( event_.key.keysym.sym == SDLK_LCTRL ) {
                lctrl_down_ = false;
            } else if( event_.key.keysym.sym == SDLK_LCTRL ) {
                rctrl_down_ = false;
            }
            break;
        case SDL_KEYDOWN:
            if( event_.key.keysym.sym == SDLK_SPACE ) {
                pause_ = !pause_;
            } else if( event_.key.keysym.sym == SDLK_F3 ) {
                bool display_F3 = Session::instance()->getBoolean("*display_F3");
                Session::instance()->setBoolean("*display_F3", !display_F3);
            } else if( event_.key.keysym.sym == SDLK_LCTRL ) {
                lctrl_down_ = true;
            } else if( event_.key.keysym.sym == SDLK_LCTRL ) {
                rctrl_down_ = true;
            } else if( event_.key.keysym.sym == SDLK_q ) {
                if( lctrl_down_ || rctrl_down_ ) {
                    quit_ = true;
                }
            } else if( event_.key.keysym.sym == SDLK_c ) {
                // center view to next robot
                PeopleGroup* g = map_view_->group();
                if( g->group().size() > 0 ) {
                    Character* p = g->getNextRobot();
                    map_view_->restoreCenterTile( p->tilePosition() );
                }
            } else if( event_.key.keysym.sym == SDLK_ESCAPE ) {
                setTool(nullptr);
            } else if( event_.key.keysym.sym == SDLK_b ) { // DEBUG ONLY
                BackGroundGenerator generator(100,60); //10,6);
                generator.execute("new_out.png");
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

    // handle event for all View(s)
    removeView(cur_dialog);
    event_handled = Camera::handleEvent();
    addView(cur_dialog);

    if( !event_handled && tool_ != nullptr ) {
        tool_->handleEvent();
    }
    return event_handled;
}

void SDLCamera::onMouseMove(int mouse_x, int mouse_y) {
    Camera::onMouseMove(mouse_x,mouse_y);
    // check on which 'tile' we are
    Position tile_pos = map_view_->onTile(mouse_x,mouse_y);
    map_view_->setTile(tile_pos.x, tile_pos.y);
}

void SDLCamera::onMouseWheelScroll(int wheel_x, int wheel_y) {
    Position position = map_view_->getCenterTile();
    Camera::onMouseWheelScroll(wheel_x,wheel_y);
    map_view_->restoreCenterTile(position);
}

void SDLCamera::setTool(SDLTool* tool) {
    tool_ = tool;
}

void SDLCamera::setMapView(MapView* view) {
    map_view_ = view;
    addView(map_view_);
}
/********************************************************************/
