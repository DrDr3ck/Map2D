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
}

void SDLTool::mousePress() {
}

void SDLTool::mouseMotion() {
}

void SDLTool::mouseRelease() {
}

/********************************************************************/

SDLBuildTool::SDLBuildTool(SDLCamera* camera, const std::string& icon_name) : SDLTool(camera) {
    surface_ = IMG_Load(icon_name.c_str());
}

SDL_Texture* SDLBuildTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        SDL_FreeSurface(surface_);
        rect_ = {100,100,64,64}; // debug
    }
    return texture_;
}

/********************************************************************/
