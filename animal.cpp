#include "animal.h"


/********************************************************************/

Animal::Animal(
    std::string name, Position tile_position, int image_id
) : DynamicItem(name, tile_position, image_id) {
}

void Animal::render(SDLCamera* camera, const SDL_Rect& rect) {
    if( direction_.y == 1 ) { // up, up left or up right
        camera->displayTexture( images_[1], &rect);
    } else if( direction_.x == -1) { // left or down left
        camera->displayTexture( images_[2], &rect);
    } else if( direction_.x == 1) { // right or down right
        camera->displayTexture( images_[3], &rect);
    } else { // down
        camera->displayTexture( images_[0], &rect);
    }
}

/********************************************************************/
