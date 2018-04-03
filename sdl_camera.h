#ifndef sdl_camera_h
#define sdl_camera_h

#include "character.h"
#include "map.h"
#include "job.h"
#include "camera.h"
#include "button.h"
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

/********************************************************************/

class MapData;
class Camera;

class MapView : public View {
public:
    MapView(SDLCamera* camera, MapData* data, PeopleGroup* group, JobMgr* manager);
    virtual ~MapView();

    virtual void do_render(Camera* camera, double delay_in_ms) override;
    virtual void handleEvent(Camera* camera) override;

    MapData* data() const { return data_; }

    void addWall(int x, int y);
    void removeWall(int x, int y);
    void addFloor(int x, int y);
    void removeFloor(int x, int y);

    bool onTile(int mouse_x, int mouse_y);
    const SDL_Rect& onTileRect() const { return ontile_rect_; }

    bool curTile(int& tile_x, int& tile_y);

protected:
    SDL_Rect getPeopleRect(Character* people) const;
    SDL_Rect getTileRect(int tile_x, int tile_y) const;

private:
    MapData* data_;
    PeopleGroup* group_;
    JobMgr* job_manager_;
    SDL_Texture* map_background_;
    SDL_Texture* window_background_;
    float scale_;
    float delta_x_;
    float delta_y_;
    float delta_speed_;
    float translate_x_;
    float translate_y_;

    int scaled_start_x_;
    int scaled_start_y_;
    int scaled_tile_size_;

    Character* selected_people_ = nullptr;

    int tile_x_;
    int tile_y_;
    SDL_Rect ontile_rect_;
};

/********************************************************************/

class SDLText {
public:
    SDLText(const std::string& text, const std::string& family = "pixel11", int font_size = 16, const SDL_Color& color = SDLText::black());
    ~SDLText();

    SDL_Texture* texture(SDL_Renderer* renderer);
    const SDL_Rect& rect() const { return rect_; }
    void set_position(int x, int y);

    static SDL_Color& black() {
        static SDL_Color black = {0, 0, 0, 255};
        return black;
    }

    static SDL_Color& red() {
        static SDL_Color red = {250, 50, 50, 255};
        return red;
    }

    static SDL_Color& white() {
        static SDL_Color white = {250, 250, 250, 255};
        return white;
    }

private:
    std::string text_;
    std::string family_;
    int size_; // font size
    SDL_Texture* texture_;
    SDL_Color color_;
    SDL_Rect rect_;
};

/********************************************************************/

class SDLTool;
class SDLButtonManager;

class SDLCamera : public Camera {
public:
    SDLCamera();
    virtual ~SDLCamera();

    virtual bool valid() const override;
    virtual void render(double delay_in_ms) override;
    virtual void handleEvent() override;
    virtual void do_quit() const override;
    virtual void onMouseMove(int x, int y) override;
    virtual void onMouseWheelScroll(int x, int y) override;

    SDLTool* tool() { return tool_; }
    void setTool(SDLTool* tool);

    void setMapView(MapView* view);

    MapView* mapView() const { return map_view_; }

    void displayTexture(SDL_Texture* texture, const SDL_Rect* rect);
    void displayText(SDLText& text, bool background=false);

    void getSize(int& screen_width, int& screen_height);
    SDL_Window* window() const { return window_; }
    SDL_Renderer* main_renderer() const { return main_renderer_; }
    const SDL_Event& event() const { return event_; }

private:
    SDL_Event event_;
    SDL_Window* window_;
    SDL_Renderer* main_renderer_;
    SDLTool* tool_;
    SDLButtonManager* manager_;
    MapView* map_view_;
};

/********************************************************************/

#endif // camera_h
