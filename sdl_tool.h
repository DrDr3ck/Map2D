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

#endif // sdl_tool_h
