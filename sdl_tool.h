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
    void setRect(SDL_Rect rect) { rect_ = rect; }

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
    SDLBuildTool(SDLCamera* camera, const std::string& icon_name, int type);
    virtual ~SDLBuildTool();

    virtual SDL_Texture* getTexture(SDL_Renderer* renderer) override;

    virtual void mousePressed(int button) override;

    int type() const { return type_; }

private:
    SDL_Surface* surface_;
    int type_;
};

class SDLUnbuildTool : public SDLBuildTool {
public:
    SDLUnbuildTool(SDLCamera* camera, const std::string& icon_name, int type);
    virtual ~SDLUnbuildTool() {}

    virtual void mousePressed(int button) override;
};

/********************************************************************/

class SDLBuildObjectTool : public SDLTool {
public:
    SDLBuildObjectTool(SDLCamera* camera, const std::string& icon_name, const std::string& object_name);
    virtual ~SDLBuildObjectTool();

    virtual SDL_Texture* getTexture(SDL_Renderer* renderer) override;

    virtual void mousePressed(int button) override;

    const std::string& objectName() const { return object_name_; }

private:
    SDL_Surface* surface_;
    std::string object_name_;
};

/********************************************************************/

class SDLExtractTool : public SDLTool {
public:
    SDLExtractTool(SDLCamera* camera, const std::string& icon_name, int nb);
    virtual ~SDLExtractTool();

    virtual SDL_Texture* getTexture(SDL_Renderer* renderer) override;

    virtual void mousePressed(int button) override;

    int nb() const { return nb_; }

private:
    SDL_Surface* surface_;
    int nb_;
};

/********************************************************************/

#endif // sdl_tool_h
