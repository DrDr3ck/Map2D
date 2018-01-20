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
    camera_->setTool(nullptr);
}

void SDLTool::handleEvent() {
    const SDL_Event& event = camera_->event();
    switch( event.type ) {
    case SDL_MOUSEMOTION:
        mouseMoved(event.motion.x, event.motion.y);
        break;
    case SDL_MOUSEBUTTONDOWN:
        mousePressed(event.button.which);
        break;
    case SDL_MOUSEBUTTONUP:
        mouseReleased(event.button.which);
        break;
    case SDL_KEYDOWN:
        std::string str(SDL_GetKeyName( event.key.keysym.sym ));
        keyPressed(str);
        break;
    }
}

void SDLTool::mousePressed(int button) {
    std::cout << "SDLTool::mousePressed " << button << std::endl;
}

void SDLTool::mouseMoved(int, int) {
    // Nothing to do for the moment
}

void SDLTool::keyPressed(const std::string& key) {
    std::cout << key << std::endl;
}

void SDLTool::mouseReleased(int button) {
    std::cout << "SDLTool::mouseReleased " << button << std::endl;
}

/********************************************************************/

SDLBuildTool::SDLBuildTool(SDLCamera* camera, const std::string& icon_name) : SDLTool(camera) {
    surface_ = IMG_Load(icon_name.c_str());
}

SDLBuildTool::~SDLBuildTool() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
}

SDL_Texture* SDLBuildTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
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
        camera()->mapView()->data()->addWall(x,y);
    }
}

/********************************************************************/

SDLUnbuildTool::SDLUnbuildTool(SDLCamera* camera, const std::string& icon_name) : SDLBuildTool(camera, icon_name) {
}

void SDLUnbuildTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->curTile(x,y) ) {
        camera()->mapView()->data()->removeWall(x,y);
    }
}

/********************************************************************/
