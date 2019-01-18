#ifndef utility_h
#define utility_h

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "logger.h"

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
     * Transforms a float into a string.
     */
    static std::string ftos(float f, int precision=6) {
        std::ostringstream stm;
        stm.precision(precision);
        stm << f ;
        return stm.str();
    }

    /*!
     * Transforms a double into a string.
     */
    static std::string dtos(double d, int precision=9) {
        std::ostringstream stm;
        stm.precision(precision);
        stm << d ;
        return stm.str();
    }

    static std::string trim(const std::string& str, const char c=' ')
    {
        size_t first = str.find_first_not_of(c);
        if (std::string::npos == first)
        {
            return str;
        }
        size_t last = str.find_last_not_of(c);
        return str.substr(first, (last - first + 1));
    }

    /*!
     * Returns a random number between min and max.
     */
    static int randint(int min, int max) {
        int r = std::rand() % (max+1-min);
        r += min;
        return r;
    }

    static const int tileSize = 64; // Tile Size

    /*!
     * \return true if \p original_str starts with \p prefix. spaces are trimmed if strict is false.
     */
    static bool startsWith(const std::string& original_str, const std::string& prefix, bool strict = false) {
        std::string str = original_str;
        if( !strict ) {
            str = trim(str);
        }
        return str.find(prefix) == 0;
    }

    static SDL_Surface* IMGLoad(const std::string& filename) {
        Logger::debug() << "Load image " << filename << Logger::endl;
        SDL_Surface* bg_surface = IMG_Load(filename.c_str());
        if(!bg_surface) {
            // handle error
            Logger::error() << "IMG_Load: " << IMG_GetError() << Logger::endl;
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
