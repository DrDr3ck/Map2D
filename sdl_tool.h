#ifndef sdl_tool_h
#define sdl_tool_h

#include <SDL2/SDL.h>
#include "camera.h"

class SDLCamera;

/********************************************************************/

class SDLTool : public Tool {
public:
    SDLTool(SDLCamera* camera);
    virtual ~SDLTool();

    void activate();
    void deactivate();

    virtual void handleEvent() override;
    virtual void mousePressed(int button) override;
    virtual void keyPressed(const std::string& key) override;
    virtual void mouseMoved(int mouse_x, int mouse_y) override;
    virtual void mouseReleased(int button) override;

    virtual SDL_Texture* getTexture(SDL_Renderer* renderer) = 0;
    const SDL_Rect& rect() const { return rect_; }

    SDLCamera* camera() const { return camera_; }

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
    virtual ~SDLBuildTool();

    virtual SDL_Texture* getTexture(SDL_Renderer* renderer) override;

    virtual void mousePressed(int button) override;

private:
    SDL_Surface* surface_;
};

class SDLUnbuildTool : public SDLBuildTool {
public:
    SDLUnbuildTool(SDLCamera* camera, const std::string& icon_name);
    virtual ~SDLUnbuildTool() {}

    virtual void mousePressed(int button) override;
};

/********************************************************************/

#endif // sdl_tool_h
