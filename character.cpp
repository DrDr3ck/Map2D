#include "character.h"
#include "sdl_camera.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "SDL2/SDL_image.h"

/********************************************************************/

DynamicItem::DynamicItem(std::string name, Position tile_position, int image_id) {
    name_ = name;
    tile_position_ = tile_position;
    pixel_position_.x = 0;
    pixel_position_.y = 0;

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
    if( !action_->spentTime(time_spent_) ) {
        action_->postAction();
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

void Character::render(SDLCamera* camera) {
    if( direction_.y == 1 ) { // up, up left or up right
        SDL_Rect rect = {64,64,0,0}; // TODO: from position
        camera->displayTexture( images_[1], &rect);
    } else if( direction_.x == -1) { // left or down left
        SDL_Rect rect = {64,64,0,0}; // TODO: from position
        camera->displayTexture( images_[2], &rect);
    } else if( direction_.x == 1) { // right or down right
        SDL_Rect rect = {64,64,0,0}; // TODO: from position
        camera->displayTexture( images_[3], &rect);
    } else { // down
        SDL_Rect rect = {0,0,64,64}; // TODO: from position
        camera->displayTexture( images_[0], &rect);
    }
}

void Character::setDirection(int x, int y) {
    direction_.x = x;
    direction_.y = y;
}

/********************************************************************/

bool NoAction::spentTime(double time_spent) {
    // no action: just do a random direction change around initial position
    if( time_spent > max_time_spent_ ) {
        character_->setDirection(Utility::randint(0,2)-1, Utility::randint(0,1));
        return false;
    }
    return true;
}

/********************************************************************/

CharacterSetLib::CharacterSetLib() {
    characters_surface_ = IMG_Load("robots.png");
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
