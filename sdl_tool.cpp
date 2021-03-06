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
    if( camera()->mapView()->getCurTilePosition(x,y) ) {
        Position position = {x,y};
        const Tile& cur = camera()->mapView()->data()->tile(x,y);
        if( type_ == WALLTOOL ) {
            if( Tile::isWall(cur) || Tile::isDoor(cur) || camera()->mapView()->data()->getObject(position) != nullptr ) {
                return;
            }
            camera()->mapView()->addWallJob(x,y);
        } else if( type_ == FLOORTOOL ) {
            if( Tile::isWall(cur) || Tile::isFloor(cur) || camera()->mapView()->data()->getObject(position) != nullptr ) {
                return;
            }
            camera()->mapView()->addFloorJob(x,y);
        } else if( type_ == DOORTOOL ) {
            if( Tile::isWall(cur) || Tile::isDoor(cur) || camera()->mapView()->data()->getObject(position) != nullptr ) {
                return;
            }
            if( !Tile::isFloor(cur) ) {
                Logger::info() << tr("First, you need to build a floor in order to put this door") << Logger::endl;
            } else {
                camera()->mapView()->addDoorJob(x,y);
            }
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

void SDLBuildObjectTool::activate() {
    CommandCenter* cc = CommandCenter::cur_command_center;
    if( cc == nullptr ) return;
    int nb = cc->countedItems(object_name_);
    if( nb == 0 ) {
        Logger::warning() << tr("No such item: please craft this item before using this tool.") << Logger::endl;
        return;
    }
    SDLTool::activate();
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
    if( map_view->getCurTilePosition(x,y) ) {
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
    if( map_view->getCurTilePosition(x,y) ) {
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
    icon_surface_ = Utility::IMGLoad(icon_name.c_str());
    Uint32 key = SDL_MapRGB(icon_surface_->format, 0, 255, 0);
    SDL_SetColorKey(icon_surface_ , SDL_TRUE, key);
    nb_ = nb;
}

SDLExtractTool::~SDLExtractTool() {
    if( icon_surface_ != nullptr ) {
        SDL_FreeSurface(icon_surface_);
    }
}

SDL_Texture* SDLExtractTool::getTexture(SDL_Renderer* renderer) {
    if( texture_ == nullptr ) {
        texture_ = SDL_CreateTextureFromSurface(renderer, icon_surface_);
        if( texture_ == nullptr ) {
            Logger::error() << "CreateRGBSurface failed: " << SDL_GetError() << Logger::endl;
        }
        SDL_SetTextureAlphaMod( texture_, 192 );
        SDL_FreeSurface(icon_surface_);
        icon_surface_ = nullptr;
    }
    return texture_;
}

void SDLExtractTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->getCurTilePosition(x,y) ) {
        camera()->mapView()->extractItemJob(x,y,nb_);
    }
}

/********************************************************************/

SDLTransformTool::SDLTransformTool(
    SDLCamera* camera,
    const std::string& icon_name,
    int type
) : SDLBuildTool(camera, icon_name, type) {
}

void SDLTransformTool::mousePressed(int button) {
    SDLTool::mousePressed(button);
    int x,y;
    if( camera()->mapView()->getCurTilePosition(x,y) ) {
        Position position = {x,y};
        const Tile& cur = camera()->mapView()->data()->tile(x,y);
        Logger::debug() << "SDLTransformTool " << type() << Logger::endl;
        if( type() == FIELDTOOL ) {
            // check that tile is grass or dirt
            if( cur.background_type() != Tile::GRASS && cur.background_type() != Tile::DIRT ) {
                return;
            }
            if( Tile::isWall(cur) || Tile::isFloor(cur) || camera()->mapView()->data()->getObject(position) != nullptr ) {
                return;
            }
            camera()->mapView()->addFieldJob(x,y);
        }
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
    if( mv->getCurTilePosition(x,y) ) {
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
    if( camera()->mapView()->getCurTilePosition(x,y) ) {
        if( type() == FOUNDATIONTOOL ) {
            camera()->mapView()->removeFoundationJob(x,y);
        }
    }
}

/********************************************************************/
