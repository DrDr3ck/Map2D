#ifndef font_h
#define font_h

#include <SDL2/SDL_ttf.h>
#include <map>

/********************************************************************/

class FontLib {
private:
  FontLib();
  ~FontLib();

public:
    static FontLib* instance();
    static void kill();

    TTF_Font* getFont(std::string family, int font_size);

    static std::string fontFamily();
    static int fontTitleSize();
    static int fontButtonSize();
    static int fontSize();

private:
    static FontLib* singleton_;
    std::map<std::string, TTF_Font*> familyToFont_;
};

/********************************************************************/

#endif // font_h
