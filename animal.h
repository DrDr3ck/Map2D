#ifndef animal_h
#define animal_h

#include "utility.h"
#include "character.h"
#include "sdl_camera.h"

/********************************************************************/

class Animal : public DynamicItem {
public:
    Animal(std::string name, Position tile_position, int image_id);
    virtual ~Animal() = default;

    virtual void render(SDLCamera* camera, const SDL_Rect& rect) override;

protected:
    int health_;
    int life_time_;
};

/********************************************************************/

#endif // character_h
