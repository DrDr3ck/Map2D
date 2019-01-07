#ifndef sdl_background_h
#define sdl_background_h

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/********************************************************************/
/*!
 * This class is used to generate background
 */
class BackGroundGenerator {
public:
    BackGroundGenerator(int width, int height);
    ~BackGroundGenerator();

    virtual void execute(const std::string& filename) const;

private:
    int width_;
    int height_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;

    std::vector<SDL_Surface*> surfaces_;
};

/********************************************************************/

#endif // sdl_background_h
