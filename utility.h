#ifndef utility_h
#define utility_h

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "logger.h"

/********************************************************************/

#define EXTRACT "extract"
#define CLEAN "clean"
#define DEMOLISHFOUNDATION "demolish_foundation"
#define BUILDWALL "build_wall"
#define BUILDFLOOR "build_floor"
#define BUILDDOOR "build_door"
#define BUILDOBJECT "build_object"
#define DEMOLISHOBJECT "demolish_object"

#define WALLTOOL 0
#define FLOORTOOL 1
#define DOORTOOL 2
#define FOUNDATIONTOOL 3
#define OBJECTTOOL 4

/********************************************************************/

struct Direction {
    int x;
    int y;
};

struct Position {
    int x;
    int y;
};

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

    /*!
     * \return true if \p original_str ends with \p suffix. spaces are trimmed if strict is false.
     */
    static bool endsWith(const std::string& original_str, const std::string& suffix, bool strict = false) {
        std::string str = original_str;
        if( !strict ) {
            str = trim(str);
        }
        if( str.length() < suffix.length() ) return false;
        if( str.find(suffix) == std::string::npos ) return false;
        return str.find(suffix) == (str.length()-suffix.length());
    }

    /*!
     * Replaces substr in original_str by \p by
     */
    static bool replace(std::string& original_str, const std::string& substr, const std::string& by) {
        int index = original_str.find(substr);
        if( index < 0 ) {
            return false;
        }
        original_str.replace(
            original_str.begin()+index,
            original_str.begin()+index+substr.length(),
            by
        );
        return true;
    }

    static SDL_Surface* IMGLoad(const std::string& filename, bool verbose=true) {
        if( verbose ) {
            Logger::debug() << "Load image " << filename << Logger::endl;
        }
        if( filename.empty() ) {
            Logger::error() << "IMG_Load(empty name): " << Logger::endl;
            return nullptr;
        }
        SDL_Surface* bg_surface = IMG_Load(filename.c_str());
        if(!bg_surface) {
            // handle error
            Logger::error() << "IMG_Load(" << filename << "): " << IMG_GetError() << Logger::endl;
        }
        return bg_surface;
    }

    static float distance(Position from, Position to) {
        int X = (to.x - from.x);
        int Y = (to.y - from.y);
        int dist_square = X*X +  Y*Y;
        return sqrt(dist_square);
    }

    static bool contains(SDL_Rect rect, int x, int y) {
        if( x < rect.x ) return false;
        if( x > rect.x+rect.w ) return false;
        if( y < rect.y ) return false;
        if( y > rect.y+rect.h ) return false;
        return true;
    }

};

/********************************************************************/

#endif // utility_h
