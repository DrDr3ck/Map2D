#ifndef sdl_camera_h
#define sdl_camera_h

#include "map.h"
#include "camera.h"
#include <SDL2/SDL.h>

class MapView : public View {
public:
    MapView(MapData* data);
    ~MapView() { data_ = nullptr; }

    virtual void do_render(Camera* camera) override;
    virtual void handleEvent(Camera* camera) override;
private:
    MapData* data_;
    SDL_Texture* background_;
    SDL_Texture* wall_;
};

class SDLCamera : public Camera {
public:
    SDLCamera();
    virtual ~SDLCamera();

    virtual bool valid() const override;

    virtual void render() override;
    virtual void handleEvent() override;

    virtual void do_quit() const override;

    SDL_Window* window() const { return window_; }
    SDL_Renderer* main_renderer() const { return main_renderer_; }
    const SDL_Event& event() const { return event_; }
private:
    SDL_Event event_;
    SDL_Window* window_;
    SDL_Renderer* main_renderer_;

};

#endif // camera_h
