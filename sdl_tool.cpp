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
    std::ignore = key;
    //Logger::debug() << "KeyPressed: " << key << Logger::endl;
}

void SDLTool::mouseReleased(int) {
}

/********************************************************************/

SDLBuildTool::SDLBuildTool(
    SDLCamera* camera,
    const std::string& icon_name,
    int type
) : SDLTool(camera) {
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
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
        SDL_SetTextureAlphaMod( texture_, 192 );
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
    }
    return texture_;
}

void SDLBuildTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->getCurTile(x,y) ) {
        if( type_ == WALLTOOL ) {
            camera()->mapView()->addWallJob(x,y);
        } else if( type_ == FLOORTOOL ) {
            camera()->mapView()->addFloorJob(x,y);
        } else if( type_ == DOORTOOL ) {
            camera()->mapView()->addDoorJob(x,y);
        }
    }
}

/********************************************************************/

SDLBuildObjectTool::SDLBuildObjectTool(
    SDLCamera* camera,
    const std::string& icon_name,
    const std::string& object_name
) : SDLTool(camera) {
    surface_ = Utility::IMGLoad(icon_name.c_str());
    object_name_ = object_name;
}

SDLBuildObjectTool::~SDLBuildObjectTool() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
}

SDL_Texture* SDLBuildObjectTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
        SDL_SetTextureAlphaMod( texture_, 192 );
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
    }
    return texture_;
}

void SDLBuildObjectTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    MapView* map_view = camera()->mapView();
    if( map_view->getCurTile(x,y) ) {
        // check that an object is not already here !!
        if( map_view->getObject(x,y) == nullptr ) {
            map_view->addObjectJob(object_name_,x,y);
        }
    }
}

/********************************************************************/

SDLUnbuildObjectTool::SDLUnbuildObjectTool(
    SDLCamera* camera,
    const std::string& icon_name
) : SDLTool(camera) {
    surface_ = Utility::IMGLoad(icon_name.c_str());
}

SDLUnbuildObjectTool::~SDLUnbuildObjectTool() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
}

SDL_Texture* SDLUnbuildObjectTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
        SDL_SetTextureAlphaMod( texture_, 192 );
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
    }
    return texture_;
}

void SDLUnbuildObjectTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    MapView* map_view = camera()->mapView();
    if( map_view->getCurTile(x,y) ) {
        // check that an object is already here !!
        if( map_view->getObject(x,y) != nullptr ) {
            map_view->removeObjectJob(x,y);
        }
    }
}

/********************************************************************/

SDLExtractTool::SDLExtractTool(
    SDLCamera* camera,
    const std::string& icon_name,
    int nb
) : SDLTool(camera) {
    surface_ = Utility::IMGLoad(icon_name.c_str());
    Uint32 key = SDL_MapRGB(surface_->format, 0, 255, 0);
    SDL_SetColorKey(surface_ , SDL_TRUE, key);
    nb_ = nb;
}

SDLExtractTool::~SDLExtractTool() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
}

SDL_Texture* SDLExtractTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
        SDL_SetTextureAlphaMod( texture_, 192 );
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
    }
    return texture_;
}

void SDLExtractTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->getCurTile(x,y) ) {
        camera()->mapView()->extractItemJob(x,y,nb_);
    }
}

/********************************************************************/

SDLCleanTool::SDLCleanTool(
    SDLCamera* camera,
    const std::string& icon_name
) : SDLTool(camera) {
    surface_ = Utility::IMGLoad(icon_name.c_str());
    Uint32 key = SDL_MapRGB(surface_->format, 0, 255, 0);
    SDL_SetColorKey(surface_ , SDL_TRUE, key);
}

SDLCleanTool::~SDLCleanTool() {
    if( surface_ != nullptr ) {
        SDL_FreeSurface(surface_);
    }
}

SDL_Texture* SDLCleanTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
        SDL_SetTextureAlphaMod( texture_, 192 );
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
    }
    return texture_;
}

void SDLCleanTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    MapView* mv = camera()->mapView();
    if( mv->getCurTile(x,y) ) {
        const Tile& cur_tile = mv->data()->tile(x,y);
        if( cur_tile.counted_items().size() > 0 ) {
            mv->cleanItemJob(x,y);
        }
    }
}

/********************************************************************/

SDLUnbuildTool::SDLUnbuildTool(SDLCamera* camera, const std::string& icon_name, int type) : SDLBuildTool(camera, icon_name, type) {
}

void SDLUnbuildTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->getCurTile(x,y) ) {
        if( type() == WALLTOOL ) {
            camera()->mapView()->removeWallJob(x,y);
        } else if( type() == FLOORTOOL ) {
            camera()->mapView()->removeFloorJob(x,y);
        } else if( type() == DOORTOOL ) {
            camera()->mapView()->removeDoorJob(x,y);
        }
    }
}

/********************************************************************/
