#ifndef sdl_camera_h
#define sdl_camera_h

#include "map.h"
#include "camera.h"
#include "button.h"
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

/********************************************************************/

class MapView : public View {
public:
    MapView(MapData* data);
    ~MapView() { data_ = nullptr; }

    virtual void do_render(Camera* camera) override;
    virtual void handleEvent(Camera* camera) override;

    bool onTile(int mouse_x, int mouse_y);
private:
    MapData* data_;
    SDL_Texture* background_;
    float scale_;
    float delta_x_;
    float delta_y_;
    float delta_speed_;
    float translate_x_;
    float translate_y_;

    int scaled_start_x_;
    int scaled_start_y_;
    int scaled_tile_size_;

    int tile_x_;
    int tile_y_;
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
    virtual void render() override;
    virtual void handleEvent() override;
    virtual void do_quit() const override;
    virtual void onMouseMove(int x, int y) override;
    virtual void onMouseWheelScroll(int x, int y) override;

    void setTool(SDLTool* tool) { tool_ = tool; }

    void setMapView(MapView* view);

    void displayTexture(SDL_Texture* texture, const SDL_Rect* rect);
    void displayText(SDLText& text);

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

class SDLQuitButton : public SDLButton {
public:
    SDLQuitButton(SDLCamera* camera, int x, int y) : SDLButton("quit.png", x,y), camera_(camera) {}
    virtual ~SDLQuitButton() {}

    virtual void activate();
private:
    SDLCamera* camera_;
};

/********************************************************************/

class SDLTool : public Tool {
public:
    SDLTool(SDLCamera* camera);
    virtual ~SDLTool();

    void activate();
    void deactivate();

    virtual void handleEvent() override;
    virtual void mousePress() override;
    virtual void mouseMotion() override;
    virtual void mouseRelease() override;

    virtual SDL_Texture* getTexture(SDL_Renderer* renderer) = 0;
    const SDL_Rect& rect() const { return rect_; }

protected:
    SDLCamera* camera_;
    //int position_x_;
    //int position_y_;
    SDL_Texture* texture_;
    SDL_Rect rect_;

};

/********************************************************************/

class SDLBuildTool : public SDLTool {
public:
    SDLBuildTool(SDLCamera* camera, const std::string& icon_name);
    virtual ~SDLBuildTool() {}

    virtual SDL_Texture* getTexture(SDL_Renderer* renderer) override;
private:
    SDL_Surface* surface_;
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

#endif // camera_h
