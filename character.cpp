#include "character.h"
#include "sdl_camera.h"
#include "path_finding.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "SDL2/SDL_image.h"

/********************************************************************/

DynamicItem::DynamicItem(std::string name, Position tile_position, int image_id) : image_id_(image_id) {
    name_ = name;
    tile_position_ = tile_position;
    pixel_position_.x = -1;
    pixel_position_.y = -1;

    for( int i=0; i < 4; i++ ) {
        images_.push_back(CharacterSetLib::instance()->getTextureFromCharacter(image_id+i));
    }

    time_spent_ = 0;
    action_ = nullptr;
    // TODO self.job = None
}

void DynamicItem::animate(double delta) {
    time_spent_ += delta;
    if( action_ == nullptr ) {
        return;
    }
    //std::cout << "Animate " << name_ << std::endl;
    if( !action_->spentTime(time_spent_) ) {
        action_->postAction();
        delete action_;
        action_ = nullptr;
        time_spent_ = 0;
    }
}

void DynamicItem::setAction(ActionBase* action, std::string description) {
    action_ = action;
    action_description_ = description;
}

/********************************************************************/

Character::Character(
    std::string name, Position tile_position, int image_id
) : DynamicItem(name, tile_position, image_id) {
}

void Character::render(SDLCamera* camera, const SDL_Rect& rect) {
    if( direction_.y == 1 ) { // up, up left or up right
        camera->displayTexture( images_[1], &rect);
    } else if( direction_.x == -1) { // left or down left
        camera->displayTexture( images_[2], &rect);
    } else if( direction_.x == 1) { // right or down right
        camera->displayTexture( images_[3], &rect);
    } else { // down
        camera->displayTexture( images_[0], &rect);
    }

    if( activity_percent_ > 0 ) {
        static int offset = 3;
        SDL_Rect activity_rect = {rect.x+offset,rect.y+offset,0,0};
        SDL_SetRenderDrawColor( camera->main_renderer(), 0, 250, 0, 255 );
        int width = activity_percent_ / 100.0 * (64-2*offset);
        activity_rect.w = width;
        activity_rect.h = 9;
        SDL_RenderFillRect( camera->main_renderer(), &activity_rect );
        SDL_SetRenderDrawColor( camera->main_renderer(), 0, 255, 128, 255 );
        activity_rect.w = 64-2*offset;
        activity_rect.h = 9;
        SDL_RenderDrawRect( camera->main_renderer(), &activity_rect );
    }
}

void Character::setDirection(int x, int y) {
    direction_.x = x;
    direction_.y = y;
}

/********************************************************************/

bool NoAction::spentTime(double time_spent) {
    // no action: just do a random direction change around initial position
    if( time_spent/1000. > max_time_spent_ ) {
        character_->setDirection(Utility::randint(0,2)-1, Utility::randint(0,1));
        return false;
    }
    return true;
}

/********************************************************************/

MoveAction::MoveAction(Character* character, std::vector<Position>& path_in_tile, int tile_size) {
    character_ = character;
    origin_ = character_->tilePosition();
    if( !character_->validPixelPosition() ) {
        character_->setPixelPosition(origin_.x*tile_size, origin_.y*tile_size);
    }
    character_->setTimeSpent(0);
    create_animation_path(path_in_tile, tile_size);
    start_time_ = std::chrono::steady_clock::now();
    speed_ = 1;
    activity_percent_ = 0;
    destination_ = path_in_tile.at( path_in_tile.size()-1 );
    is_finished_ = false;
}

bool MoveAction::spentTime(double /*time_spent*/) {
    Position new_pixel_pos = next_position_;
    Position old_pixel_pos = prev_position_;
    if( new_pixel_pos.x == old_pixel_pos.x ) {
        character_->setDirection(0, character_->direction().y );
    } else if( new_pixel_pos.x < old_pixel_pos.x ) {
        character_->setDirection(-1, character_->direction().y );
    } else if( new_pixel_pos.x > old_pixel_pos.x ) {
        character_->setDirection(1, character_->direction().y );
    }
    if( new_pixel_pos.y == old_pixel_pos.y ) {
        character_->setDirection(character_->direction().x, 0 );
    } else if( new_pixel_pos.y < old_pixel_pos.y ) {
        character_->setDirection(character_->direction().x, 1 );
    } else if( new_pixel_pos.y > old_pixel_pos.y ) {
        character_->setDirection(character_->direction().x, -1 );
    }
    new_pixel_pos = get_position_in_pixel();
    character_->setPixelPosition(new_pixel_pos.x, new_pixel_pos.y);
    if( is_finished_ ) {
        return false;
    }
    return true;
}

void MoveAction::postAction() {
    character_->setTilePosition( destination_ );
    character_->setPixelPosition(-1,-1); // invalidate pixel position
    character_->setTimeSpent(0);
}

void MoveAction::create_animation_path(std::vector<Position>& path_in_tile, int tile_size) {
    path_in_pixel_.clear();
    path_in_tile.erase(path_in_tile.begin());
    if( character_->validPixelPosition() ) {
        path_in_pixel_.push_back( character_->pixelPosition() );
    }
    unsigned int i=0;
    for( auto position : path_in_tile ) {
        i++;
        Position cur = { position.x*tile_size, position.y*tile_size };
        if( i < path_in_tile.size() ) {
            // randomize a little bit by adding some pixels in the move
            cur.x = cur.x + Utility::randint(-2,8);
            cur.y = cur.y + Utility::randint(-2,8);
        }
        path_in_pixel_.push_back( cur );
    }
}

Position MoveAction::get_position_in_pixel() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    double cur_time = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_).count();
    cur_time *= speed_;
    cur_time = cur_time / 1000000.; // time spent in seconds
    activity_percent_ = int(100.0*cur_time/(path_in_pixel_.size()-1));
    if( cur_time >= (path_in_pixel_.size()-1) ) {
        is_finished_ = true;
        return path_in_pixel_.back();
    }
    int base = int(cur_time);
    Position prev = path_in_pixel_.at(base);
    int x1 = prev.x;
    int y1 = prev.y;
    Position cur_position = {
        int(std::round(x1/64.)),
        int(std::round(y1/64.)) };
    character_->setTilePosition( cur_position );
    prev_position_ = cur_position;
    Position next = path_in_pixel_.at(base+1);
    int x2 = next.x;
    int y2 = next.y;
    next_position_ = { int(std::round(x2/64.)), int(std::round(y2/64.))  };
    double factor = cur_time - base;
    Position cur_in_pixel;
    cur_in_pixel.x = int(x1 + (x2-x1)*factor);
    cur_in_pixel.y = int(y1 + (y2-y1)*factor);
    return cur_in_pixel;
}

/********************************************************************/

BuildAction::BuildAction(
    GameBoard* game_board, Character* people, Job* job, int tile_size
) {
    game_board_ = game_board;
    people_ = people;
    job_ = job;
    tile_size_ = tile_size;
    action_ = nullptr;
    isValid_ = true;
}

BuildAction::~BuildAction() {
}

void BuildAction::preAction() {
    if( people_->tilePosition().x == job_->tilePosition().x && people_->tilePosition().y == job_->tilePosition().y ) {
        return;
    }
    PathFinding path(game_board_->data());
    Position end_position = job_->tilePosition();
    std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);

    if( positions.size() == 0 ) {
        isValid_ = false;
        job_->reset();
    } else {
        action_ = new MoveAction(people_, positions, tile_size_);
    }
}

bool BuildAction::spentTime(double time_spent) {
    if( !isValid_ ) return false;
    if( action_ != nullptr ) {
        if( !action_->spentTime(time_spent) ) {
            action_->postAction();
            delete action_;
            action_ = nullptr;
            start_time_ = std::chrono::steady_clock::now();
            if( !game_board_->jobManager()->findJobAt(job_->tilePosition() ) ) {
                std::cout << "Job cancel" << std::endl;
                return false; // job has been cancel
            }
            // set direction so that people can 'work'
            people_->setDirection(people_->direction().x, 1);
        }
    } else {
        // spent time on construction...
        std::chrono::steady_clock::time_point cur_time = std::chrono::steady_clock::now();
        double delay_anim_us = std::chrono::duration_cast<std::chrono::microseconds>(cur_time - start_time_).count();
        double delta_time = delay_anim_us/1000.;
        people_->setActivityPercent( std::min(100,int(100.0*delta_time/job_->buildTime())) );
        if( delta_time > job_->buildTime() ) {
            return false;
        }
    }
    return true;
}

void BuildAction::postAction() {
    people_->setActivityPercent(0);
    if( !isValid_ ) return;
    Position position = job_->tilePosition();
    if( game_board_->jobManager()->findJobAt(position) ) {
        game_board_->jobManager()->cancelJob(position);
        if( job_->name() == DEMOLISHWALL ) {
            game_board_->data()->removeWall(position.x,position.y);
        } else if( job_->name() == BUILDWALL ) {
            game_board_->data()->addWall(position.x,position.y);
        } else if( job_->name() == BUILDFLOOR ) {
            game_board_->data()->addFloor(position.x,position.y);
        } else if( job_->name() == DEMOLISHFLOOR ) {
            game_board_->data()->removeFloor(position.x,position.y);
        }
    }
}

/********************************************************************/

CharacterSetLib::CharacterSetLib() {
    std::cout << "CharacterSetLib" << std::endl;
    characters_surface_ = Utility::IMGLoad("robots.png");

    Uint32 key = SDL_MapRGB(characters_surface_->format, 0, 255, 0);
    SDL_SetColorKey(characters_surface_, SDL_TRUE, key);

    if( characters_surface_ == nullptr ) {
        std::cout << "cannot initialize CharacterSetLib" << std::endl;
    }
}

CharacterSetLib::~CharacterSetLib() {
    SDL_FreeSurface(characters_surface_);
}

CharacterSetLib* CharacterSetLib::instance() {
    if( singleton_ == nullptr ) {
        std::cout << "creating CharacterSetLib singleton" << std::endl;
        singleton_ =  new CharacterSetLib();
    }
    return singleton_;
}

void CharacterSetLib::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        std::cout << "destroying CharacterSetLib singleton" << std::endl;
        singleton_ = nullptr;
    }
}

void CharacterSetLib::init(SDL_Renderer* renderer) {
    static int tileSize = 64;
    for( int x=0; x<4; x++ ) {
        for( int y=0; y<4; y++ ) {
            int id = x+y*4;
            SDL_Rect source;
            source.x = x * tileSize;
            source.y = y * tileSize;
            source.w = tileSize;
            source.h = tileSize;

            SDL_Surface* surf_dest = SDL_CreateRGBSurface(0, tileSize, tileSize, 32, 0, 0, 0, 0);

            SDL_Rect dest;
            dest.x = 0;
            dest.y = 0;
            dest.w = tileSize;
            dest.h = tileSize;

            SDL_Surface* surf_source = CharacterSetLib::instance()->characters();

            SDL_BlitSurface(surf_source,
                            &source,
                            surf_dest,
                            &dest);

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf_dest);
            CharacterSetLib::instance()->mapOfCharacters()[id] = texture;
        }
    }
}

/*!
 * \return the texture from the given \p tile.
 */
SDL_Texture* CharacterSetLib::getTextureFromCharacter(int id) {
    auto map_of_tiles = CharacterSetLib::instance()->mapOfCharacters();

    if( map_of_tiles.find(id) != map_of_tiles.end()) {
        return map_of_tiles[id];
    }

    return nullptr;
}

// Initialize singleton_ to nullptr
CharacterSetLib* CharacterSetLib::singleton_ = nullptr;

/********************************************************************/

PeopleGroup::PeopleGroup() {
}

PeopleGroup::~PeopleGroup() {
    group_.clear();
}

void PeopleGroup::animate(GameBoard* board, double delta_ms) {
    for( auto people : group_ ) {
        if( people->action() == nullptr ) {
            Job* job = board->jobManager()->getFirstAvailableJob();
            if( job != nullptr ) {
                std::cout << job->name() << std::endl;
                if( job->name() == BUILDWALL ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job, 64), "building a wall" );
                    people->action()->preAction();
                } else if( job->name() == DEMOLISHWALL ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job, 64), "demolishing a wall" );
                    people->action()->preAction();
                } else if( job->name() == BUILDFLOOR ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job, 64), "building a foundation" );
                    people->action()->preAction();
                } else if( job->name() == DEMOLISHFLOOR ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job, 64), "demolishing a foundation" );
                    people->action()->preAction();
                } else if( job->name() == "build_object" ) {
                    // TODO
                    //job.dynamic_item = people
                    //people.action = BuildObjectAction(self, people, job, self.tile_size)
                    //people.action.preAction()
                }
            }
        }
        if( people->action() == nullptr ) {
            new NoAction(people); // set action on people is done in constructor of NoAction
        }
        people->animate(delta_ms);
    }
}

std::vector<Character*>& PeopleGroup::group() {
    return group_;
}

void PeopleGroup::add(Character* people) {
    group_.push_back(people);
}

/********************************************************************/
