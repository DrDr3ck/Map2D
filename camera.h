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
};

class MapView : public View {
public:
    MapView(MapData* data);
    ~MapView() { data_ = nullptr; }

    virtual void do_render(SDL_Renderer* main_renderer) override;
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

    SDL_Window* window() const { return window_; }
    SDL_Renderer* main_renderer() const { return main_renderer_; }

private:
    SDL_Window* window_;
    SDL_Renderer* main_renderer_;
    std::list<View*> views_;
};

#endif // camera_h
