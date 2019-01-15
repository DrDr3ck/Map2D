#ifndef sdl_background_h
#define sdl_background_h

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/********************************************************************/

class Biome {
public:
    Biome(const std::string& type);
    ~Biome();

    const std::vector<int>& heights() const {
        return heights_;
    }
    const std::vector<int>& surface_columns() const {
        return surface_columns_;
    }
    const std::vector<int>& surface_rows() const {
        return surface_rows_;
    }

    bool is_valid() const;

    const std::string& type() const {
        return type_;
    }

    SDL_Surface* surface(int index) {
        return surfaces_[index];
    }

    int getType(float value) const;

private:
    std::string type_;
    std::vector<SDL_Surface*> surfaces_;
    std::vector<int> heights_;
    // nb columns/row for each surface
    // if surface is composed of 6 images (3 columns, 2 rows for instance)
    std::vector<int> surface_columns_;
    std::vector<int> surface_rows_;
};

/********************************************************************/

/*!
 * This class is used to generate background
 */
class BackGroundGenerator {
public:
    BackGroundGenerator(int width, int height, Biome* biome=nullptr);
    ~BackGroundGenerator();

    virtual void execute(const std::string& filename, float** noise_map = nullptr) const;

private:
    int width_;
    int height_;
    Biome* biome_;
};

/********************************************************************/

#endif // sdl_background_h
