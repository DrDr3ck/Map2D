#ifndef texture_mgr_h
#define texture_mgr_h

#include <SDL2/SDL.h>
#include <map>
#include <string>

/********************************************************************/

class TextureMgr {
private:
  TextureMgr() {}
  ~TextureMgr() {}

public:
    static TextureMgr* instance();
    static void kill();

    void loadAllItems(SDL_Renderer* renderer);
    //void loadAllObjects(SDL_Renderer* renderer);

    SDL_Texture* getItemTexture(const std::string& name);

private:
    static TextureMgr* singleton_;
    std::map<std::string, SDL_Texture*> textured_items_;
};

/********************************************************************/

#endif // texture_mgr_h
