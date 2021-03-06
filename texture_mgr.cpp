#include "texture_mgr.h"
#include "logger.h"
#include "utility.h"
#include <dirent.h>

/********************************************************************/

// Initialize singleton_ to nullptr
TextureMgr* TextureMgr::singleton_ = nullptr;

TextureMgr* TextureMgr::instance() {
    if( singleton_ == nullptr ) {
        singleton_ = new TextureMgr();
        Logger::debug() << "creating TextureMgr singleton" << Logger::endl;
    }
    return singleton_;
}

void TextureMgr::kill() {
    if( singleton_ != nullptr ) {
        Logger::debug() << "destroying TextureMgr singleton" << Logger::endl;
        delete singleton_;
        singleton_ = nullptr;
    }
}

void TextureMgr::loadAllItems(SDL_Renderer* renderer) {
    // get all files that end with 'xxx_item.png' and create an entry named xxx
#ifdef _WIN32
    DIR* dir = opendir(".\\items\\");
#else
    DIR* dir = opendir("./items/");
#endif
    std::string suffix("_item.png");
    struct dirent* file = nullptr;
    while((file = readdir(dir)) != nullptr ) {
        std::string filename(file->d_name);
        if( Utility::endsWith(filename, suffix) ) {
#ifdef _WIN32
            std::string full_filename(".\\items\\");
#else
            std::string full_filename("./items/");
#endif
            full_filename.append(filename);
            SDL_Surface* surface = Utility::IMGLoad(full_filename);
            if( surface == nullptr ) {
                Logger::error() << "Cannot open " << filename << Logger::endl;
                continue;
            }

            // Transparency with green color
            Uint32 key = SDL_MapRGB(surface->format, 0, 255, 0);
            SDL_SetColorKey(surface , SDL_TRUE, key);

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            int index = filename.find(suffix);
            std::string item_name = filename.substr(0, index);
            textured_items_.insert(std::pair<std::string, SDL_Texture*>(item_name, texture));
        }
    }
}

SDL_Texture* TextureMgr::getItemTexture(const std::string& name) {
    std::map<std::string, SDL_Texture*>::iterator it = textured_items_.find(name);
    if( it == textured_items_.end() ) {
        return nullptr;
    }
    return it->second;
}
