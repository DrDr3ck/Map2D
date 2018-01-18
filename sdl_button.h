#ifndef sdl_button_h
#define sdl_button_h

#include "button.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

class SDLCamera;
class SDLTool;

/********************************************************************/

class SDLButtonManager : public ButtonManager {
public:
    SDLButtonManager() : ButtonManager() {}
    virtual ~SDLButtonManager() {}

    virtual void do_render(Camera* camera);
    virtual void handleEvent(Camera* camera) override;
};

/********************************************************************/

class SDLButton : public Button {
public:
    SDLButton(std::string icon_name, int x, int y);
    virtual ~SDLButton();

    virtual void activate() override;
    virtual void deactivate() override;

    virtual void setSize(int w, int h) override;
    virtual void setPosition(int x,int y) override;

    SDL_Texture* getTexture(SDL_Renderer* renderer);
    const SDL_Rect& rect() const { return rect_; }

private:
    SDL_Surface* surface_;
    SDL_Texture* texture_;
    SDL_Rect rect_;
};

/********************************************************************/

class SDLButtonMenu : public SDLButton {
public:
    SDLButtonMenu(MenuButton* menu, std::string name, int x, int y) : SDLButton(name, x,y), menu_(menu) {}
    virtual ~SDLButtonMenu() {}

    virtual void activate() override;
    virtual void deactivate() override;
private:
    MenuButton* menu_;
};

/********************************************************************/

class SDLQuitButton : public SDLButton {
public:
    SDLQuitButton(SDLCamera* camera, int x, int y) : SDLButton("quit.png", x,y), camera_(camera) {}
    virtual ~SDLQuitButton() {}

    virtual void activate();
private:
    SDLCamera* camera_;
};

/********************************************************************/

class SDLToolButton : public SDLButton {
public:
    SDLToolButton(SDLTool* tool, std::string icon_name, int x, int y);
    virtual ~SDLToolButton();

    virtual void activate() override;
    virtual void deactivate() override;

private:
    SDLTool* tool_;
};

/********************************************************************/

#endif // sdl_button_h
