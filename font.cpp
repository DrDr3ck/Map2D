#include "font.h"
#include "map.h"

#include <iostream>

/********************************************************************/

FontManager::FontManager() {
}

FontManager::~FontManager() {
    for( auto it=familyToFont_.begin(); it!=familyToFont_.end(); it++ ){
        TTF_Font* font = it->second;
        TTF_CloseFont(font);
    }
}

FontManager* FontManager::instance() {
    if( singleton_ == nullptr ) {
        std::cout << "creating FontManager singleton" << std::endl;
        singleton_ =  new FontManager();
    }
    return singleton_;
}

void FontManager::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        std::cout << "destroying FontManager singleton" << std::endl;
        singleton_ = nullptr;
    }
}

TTF_Font* FontManager::getFont(std::string family, int font_size) {
    std::string key = family;
    key.append(" ");
    key.append(Utility::itos(font_size));
    if(familyToFont_.find(key) == familyToFont_.end()) {
        TTF_Font* font = TTF_OpenFont(family.append(".ttf").c_str(), font_size);
        familyToFont_[key] = font;
    }

    return familyToFont_[key];
}

// Initialize singleton_ to nullptr
FontManager* FontManager::singleton_ = nullptr;

/********************************************************************/
