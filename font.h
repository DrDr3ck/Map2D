#ifndef font_h
#define font_h

#include <SDL2/SDL_ttf.h>
#include <map>

/********************************************************************/

class FontManager {
private:
  FontManager();
  ~FontManager();

public:
    static FontManager* instance();
    static void kill();

    TTF_Font* getFont(std::string family, int font_size);

private:
    static FontManager* singleton_;
    std::map<std::string, TTF_Font*> familyToFont_;
};

/********************************************************************/

#endif // map_h
