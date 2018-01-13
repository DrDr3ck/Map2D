#ifndef sdl_camera_h
#define sdl_camera_h

#include "map.h"
#include "camera.h"
#include "button.h"
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/********************************************************************/

class MapView : public View {
public:
    MapView(MapData* data);
    ~MapView() { data_ = nullptr; }

    virtual void do_render(Camera* camera) override;
    virtual void handleEvent(Camera* camera) override;
private:
    MapData* data_;
    SDL_Texture* background_;
    float scale_;
    float delta_x_;
    float delta_y_;
    float delta_speed_;
};

/********************************************************************/

class SDLTool;
class SDLButtonManager;

class SDLCamera : public Camera {
public:
    SDLCamera();
    virtual ~SDLCamera();

    virtual bool valid() const override;
    virtual void render() override;
    virtual void handleEvent() override;
    virtual void do_quit() const override;

    void setTool(SDLTool* tool) { tool_ = tool; }

    void displayTexture(SDL_Texture* texture, const SDL_Rect* rect);
    void getSize(int& screen_width, int& screen_height);
    SDL_Window* window() const { return window_; }
    SDL_Renderer* main_renderer() const { return main_renderer_; }
    const SDL_Event& event() const { return event_; }

private:
    SDL_Event event_;
    SDL_Window* window_;
    SDL_Renderer* main_renderer_;
    TTF_Font* font_;
    SDLTool* tool_;
    SDLButtonManager* manager_;
};

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
    SDLButton(std::string name, int x, int y);
    virtual ~SDLButton();

    virtual void activate() override;
    virtual void deactivate() override;

    SDL_Texture* getTexture(SDL_Renderer* renderer);
    const SDL_Rect& rect() const { return rect_; }

private:
    SDL_Surface* surface_;
    SDL_Texture* texture_;
    SDL_Rect rect_;
};

/********************************************************************/

class SDLQuitButton : public SDLButton {
public:
    SDLQuitButton(SDLCamera* camera, int x, int y) : SDLButton("quit.bmp", x,y), camera_(camera) {}
    virtual ~SDLQuitButton() {}

    virtual void activate();
private:
    SDLCamera* camera_;
};

/********************************************************************/

class SDLTool : public Tool {
public:
    SDLTool(SDLCamera* camera) : Tool(), camera_(camera) {}
    virtual ~SDLTool() {}

    virtual void handleEvent() override;
    virtual void mousePress() override;
    virtual void mouseMotion() override;
    virtual void mouseRelease() override;

private:
    SDLCamera* camera_;
    int position_x_;
    int position_y_;
};

/********************************************************************/

#endif // camera_h
