#ifndef utility_h
#define utility_h

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/********************************************************************/

#define BUILDWALL "build"
#define DEMOLISHWALL "demolish"
#define BUILDFLOOR "build_floor"
#define DEMOLISHFLOOR "demolish_floor"

#define WALLTOOL 0
#define FLOORTOOL 1
#define OBJECTTOOL 2

/********************************************************************/

class Utility {
public:
    /*!
     * Transforms an integer into a string.
     */
    static std::string itos(int i) {
        std::ostringstream stm;
        stm << i ;
        return stm.str();
    }

    /*!
     * Returns a random number between min and max.
     */
    static int randint(int min, int max) {
        int r = std::rand() % (max+1-min);
        r += min;
        return r;
    }

    static SDL_Surface* IMGLoad(const std::string& filename) {
        std::cout << "Load image " << filename << std::endl;
        SDL_Surface* bg_surface = IMG_Load(filename.c_str());
        if(!bg_surface) {
            // handle error
            std::cout << "IMG_Load: " << IMG_GetError() << std::endl;
        }
        return bg_surface;
    }

};

struct Direction {
    int x;
    int y;
};

struct Position {
    int x;
    int y;
};

/********************************************************************/

#endif // utility_h
