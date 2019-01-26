#include "character.h"
#include "sdl_camera.h"
#include "path_finding.h"
#include "logger.h"
#include "translator.h"
#include "action.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "SDL2/SDL_image.h"

/********************************************************************/

Item::Item(std::string name, Position tile_position, int image_id) : image_id_(image_id) {
    name_ = name;
    tile_position_ = tile_position;

    for( int i=0; i < 4; i++ ) {
        images_.push_back(CharacterSetLib::instance()->getTextureFromCharacter(image_id+i));
    }
}

/********************************************************************/

DynamicItem::DynamicItem(std::string name, Position tile_position, int image_id) : Item(name, tile_position, image_id) {
    pixel_position_.x = -1;
    pixel_position_.y = -1;

    time_spent_ = 0;
    action_ = nullptr;
    // TODO self.job = None
}

void DynamicItem::animate(double delta) {
    time_spent_ += delta;
    if( action_ == nullptr ) {
        return;
    }
    //Logger::debug() << "Animate " << name_ << Logger::endl;
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
        int width = activity_percent_ / 100.0 * (Utility::tileSize-2*offset);
        activity_rect.w = width;
        activity_rect.h = 9;
        SDL_RenderFillRect( camera->main_renderer(), &activity_rect );
        SDL_SetRenderDrawColor( camera->main_renderer(), 0, 255, 128, 255 );
        activity_rect.w = Utility::tileSize-2*offset;
        activity_rect.h = 9;
        SDL_RenderDrawRect( camera->main_renderer(), &activity_rect );
    }
}

void Character::setDirection(int x, int y) {
    direction_.x = x;
    direction_.y = y;
}

/********************************************************************/

CharacterSetLib::CharacterSetLib() {
    characters_surface_ = Utility::IMGLoad("robots.png");

    if( characters_surface_ == nullptr ) {
        Logger::error() << "cannot initialize CharacterSetLib" << Logger::endl;
    }
}

CharacterSetLib::~CharacterSetLib() {
    SDL_FreeSurface(characters_surface_);
}

CharacterSetLib* CharacterSetLib::instance() {
    if( singleton_ == nullptr ) {
        Logger::debug() << "creating CharacterSetLib singleton" << Logger::endl;
        singleton_ =  new CharacterSetLib();
    }
    return singleton_;
}

void CharacterSetLib::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        Logger::debug() << "destroying CharacterSetLib singleton" << Logger::endl;
        singleton_ = nullptr;
    }
}

void CharacterSetLib::init(SDL_Renderer* renderer) {
    for( int x=0; x<4; x++ ) {
        for( int y=0; y<4; y++ ) {
            int id = x+y*4;
            SDL_Rect source;
            source.x = x * Utility::tileSize;
            source.y = y * Utility::tileSize;
            source.w = Utility::tileSize;
            source.h = Utility::tileSize;

            SDL_Surface* surf_dest = SDL_CreateRGBSurface(0, Utility::tileSize, Utility::tileSize, 32, 0, 0, 0, 0);

            SDL_Rect dest;
            dest.x = 0;
            dest.y = 0;
            dest.w = Utility::tileSize;
            dest.h = Utility::tileSize;

            SDL_Surface* surf_source = CharacterSetLib::instance()->characters();

            SDL_BlitSurface(surf_source,
                            &source,
                            surf_dest,
                            &dest);

            // Transparency with green color
            Uint32 key = SDL_MapRGB(surf_dest->format, 0, 255, 0);
            SDL_SetColorKey(surf_dest , SDL_TRUE, key);

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
                Logger::debug() << job->name() << Logger::endl;
                if( job->name() == BUILDWALL ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job), "building a wall" );
                    people->action()->preAction();
                } else if( job->name() == DEMOLISHWALL ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job), "demolishing a wall" );
                    people->action()->preAction();
                } else if( job->name() == BUILDFLOOR ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job), "building a foundation" );
                    people->action()->preAction();
                } else if( job->name() == DEMOLISHFLOOR ) {
                    job->takeJob(people);
                    people->setAction( new BuildAction(board, people, job), "demolishing a foundation" );
                    people->action()->preAction();
                } else if( job->name() == EXTRACT ) {
                    job->takeJob(people);
                    people->setAction( new ExtractAction(board, people, job), "extraction" );
                    people->action()->preAction();
                } else if( job->name() == "build_object" ) {
                    // TODO
                    //job->takeJob(people);
                    //BuildObjectJob* bjob = static_cast<BuildObjectJob*>(job);
                    //people->setAction( new BuildObjectAction(board, people, job), bjob->object_name() );
                    //people->action()->preAction();
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
