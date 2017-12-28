#ifndef camera_h
#define camera_h

#include "map.h"
#include <SDL2/SDL.h>
#include <list>

class View {
public:
    View() {}
    virtual ~View() {}

    virtual void do_render(SDL_Renderer* main_renderer) = 0;
    virtual void handleEvent(const SDL_Event& event) = 0;
};

class MapView : public View {
public:
    MapView(MapData* data);
    ~MapView() { data_ = nullptr; }

    virtual void do_render(SDL_Renderer* main_renderer) override;
    virtual void handleEvent(const SDL_Event& event) override;
private:
    MapData* data_;
    SDL_Texture* background_;
};

class Camera {
public:
    Camera();
    ~Camera();

    void addView(View* view);
    void removeView(View* view);

    void render();
    void handleEvent(const SDL_Event& event);

    SDL_Window* window() const { return window_; }
    SDL_Renderer* main_renderer() const { return main_renderer_; }

    bool quit() const { return quit_; }

private:
    SDL_Window* window_;
    SDL_Renderer* main_renderer_;
    std::list<View*> views_;
    bool pause_;
    bool quit_;
};

#endif // camera_h
