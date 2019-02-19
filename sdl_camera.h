#ifndef sdl_camera_h
#define sdl_camera_h

#include "character.h"
#include "map.h"
#include "job.h"
#include "camera.h"
#include "button.h"
#include "font.h"
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
    virtual bool handleEvent(Camera* camera) override;

    MapData* data() const { return data_; }

    void addWallJob(int x, int y);
    void removeWallJob(int x, int y);
    void addFloorJob(int x, int y);
    void removeFloorJob(int x, int y);
    void extractItemJob(int x, int y, int nb=1);
    void cleanItemJob(int x, int y);
    void addObjectJob(const std::string& object_name, int x, int y);

    Object* getObject(int x,int y) const;

    Position onTile(int mouse_x, int mouse_y) const;
    void setTile(int tile_x, int tile_y);
    const SDL_Rect& onTileRect() const { return ontile_rect_; }

    bool getCurTile(int& tile_x, int& tile_y);

    Position getCenterTile() const;
    void restoreCenterTile(Position position);

    PeopleGroup* group() const;

    bool store(const BasicItem& item, Position tile_position);

    static MapView* cur_map;

protected:
    SDL_Rect getPeopleRect(Character* people) const;
    SDL_Rect getTileRect(int tile_x, int tile_y) const;

    void renderObjects(SDLCamera* sdl_camera, std::string& tile_text);
    void renderJobs(SDLCamera* sdl_camera);
    void renderGroup(SDLCamera* sdl_camera);

private:
    MapData* data_;
    PeopleGroup* group_;
    JobMgr* job_manager_;
    SDL_Texture* map_background_;
    SDL_Texture* window_background_;
    SDLCamera* camera_;
    float scale_;
    float delta_x_;
    float delta_y_;
    float delta_speed_;
    float translate_x_;
    float translate_y_;

    int center_x_;
    int center_y_;

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
    SDLText(
        const std::string& text, const std::string& family, int font_size = FontLib::fontSize(),
        const SDL_Color& color = SDLText::black(), const SDL_Color& background_color = SDLText::white()
    );
    ~SDLText();

    SDL_Texture* texture(SDL_Renderer* renderer);
    void releaseTexture();
    const SDL_Rect& rect() const { return rect_; }
    void set_position(int x, int y);

    void setBackgroundColor(const SDL_Color& bgcolor);
    const SDL_Color& getBackgroundColor() const;

    static SDL_Color& black() {
        static SDL_Color black = {0, 0, 0, 255};
        return black;
    }

    static SDL_Color& red() {
        static SDL_Color red = {250, 50, 50, 255};
        return red;
    }

    static SDL_Color& yellow() { // royal
        static SDL_Color red = {250, 218, 94, 255};
        return red;
    }

    static SDL_Color& green() {
        static SDL_Color red = {0, 128, 0, 255};
        return red;
    }

    static SDL_Color& blue() {
        static SDL_Color blue = {50, 50, 250, 255};
        return blue;
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
    SDL_Color background_color_;
};

/********************************************************************/

class SDLTool;
class SDLButtonManager;
class SDLButton;

class SDLCamera : public Camera {
public:
    SDLCamera(int width, int height);
    virtual ~SDLCamera();

    void init();

    virtual bool valid() const override;
    virtual void render(double delay_in_ms) override;
    virtual void handleEvent() override;
    virtual void onMouseMove(int x, int y) override;
    virtual void onMouseWheelScroll(int x, int y) override;

    SDLTool* tool() { return tool_; }
    void setTool(SDLTool* tool);

    void setMapView(MapView* view);

    MapView* mapView() const { return map_view_; }

    void displayTexture(SDL_Texture* texture, const SDL_Rect* rect, const SDL_Rect* dest = NULL);
    void displayText(SDLText& text, bool background=false, bool croppable=false);
    void displayButton(SDLButton* button, int offset_x=0, int offset_y=0);

    void getSize(int& screen_width, int& screen_height);
    SDL_Window* window() const { return window_; }
    SDL_Renderer* main_renderer() const { return main_renderer_; }
    const SDL_Event& event() const { return event_; }

    static void openOptionsDialog();

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
