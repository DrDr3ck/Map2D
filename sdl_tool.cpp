#include "sdl_tool.h"

#include <iostream>
#include <math.h>
#include <string>

#include "font.h"
#include "sdl_camera.h"
#include "sdl_button.h"

/********************************************************************/

SDLTool::SDLTool(
    SDLCamera* camera
) : Tool(), camera_(camera), texture_(nullptr) {
}

SDLTool::~SDLTool() {
    if( texture_ != nullptr ) {
        SDL_DestroyTexture(texture_);
    }
}

void SDLTool::activate() {
    camera_->setTool(this);
}

void SDLTool::deactivate() {
    if( camera_->tool() == this ) {
        camera_->setTool(nullptr);
    }
}

void SDLTool::handleEvent() {
    const SDL_Event& event = camera_->event();
    switch( event.type ) {
    case SDL_MOUSEMOTION:
        mouseMoved(event.motion.x, event.motion.y);
        break;
    case SDL_MOUSEBUTTONDOWN:
        mousePressed(event.button.button);
        break;
    case SDL_MOUSEBUTTONUP:
        mouseReleased(event.button.button);
        break;
    case SDL_KEYDOWN:
        std::string str(SDL_GetKeyName( event.key.keysym.sym ));
        keyPressed(str);
        break;
    }
}

void SDLTool::mousePressed(int) {
}

void SDLTool::mouseMoved(int, int) {
    // Nothing to do for the moment
}

void SDLTool::keyPressed(const std::string& key) {
    std::cout << "KeyPressed: " << key << std::endl;
}

void SDLTool::mouseReleased(int) {
}

/********************************************************************/

SDLBuildTool::SDLBuildTool(SDLCamera* camera, const std::string& icon_name, int type) : SDLTool(camera) {
    surface_ = Utility::IMGLoad(icon_name.c_str());
    type_ = type;
}

SDLBuildTool::~SDLBuildTool() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
}

SDL_Texture* SDLBuildTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        SDL_SetTextureAlphaMod( texture_, 192 );
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
        rect_ = {100,100,64,64}; // debug
    }
    return texture_;
}

void SDLBuildTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->curTile(x,y) ) {
        if( type_ == WALLTOOL ) {
            camera()->mapView()->addWall(x,y);
        } else if( type_ == FLOORTOOL ) {
            camera()->mapView()->addFloor(x,y);
        }
    }
}

/********************************************************************/

SDLUnbuildTool::SDLUnbuildTool(SDLCamera* camera, const std::string& icon_name, int type) : SDLBuildTool(camera, icon_name, type) {
}

void SDLUnbuildTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->curTile(x,y) ) {
        if( type() == WALLTOOL ) {
            camera()->mapView()->removeWall(x,y);
        } else if( type() == FLOORTOOL ) {
            camera()->mapView()->removeFloor(x,y);
        }
    }
}

/********************************************************************/
