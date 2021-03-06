#include "font.h"
#include "map.h"
#include "logger.h"
#include "session.h"

#include <iostream>

/********************************************************************/

FontLib::FontLib() {
}

FontLib::~FontLib() {
    for( auto it=familyToFont_.begin(); it!=familyToFont_.end(); it++ ){
        TTF_Font* font = it->second;
        TTF_CloseFont(font);
    }
}

FontLib* FontLib::instance() {
    if( singleton_ == nullptr ) {
        Logger::debug() << "creating FontLib singleton" << Logger::endl;
        singleton_ =  new FontLib();
    }
    return singleton_;
}

void FontLib::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        Logger::debug() << "destroying FontLib singleton" << Logger::endl;
        singleton_ = nullptr;
    }
}

TTF_Font* FontLib::getFont(std::string family, int font_size) {
    std::string key = family;
    key.append(" ");
    key.append(Utility::itos(font_size));
    if(familyToFont_.find(key) == familyToFont_.end()) {
        TTF_Font* font = TTF_OpenFont(family.append(".ttf").c_str(), font_size);
        if( font == nullptr ) {
            font = TTF_OpenFont("pixel11.ttf", font_size);
        }
        familyToFont_[key] = font;
    }

    return familyToFont_[key];
}

std::string FontLib::fontFamily() {
    return Session::instance()->getString("*font*family", "pixel11");
}

int FontLib::fontTitleSize() {
    return Session::instance()->getInteger("*font*title*size", 14);
}

int FontLib::fontButtonSize() {
    return Session::instance()->getInteger("*font*button*size", 12);
}

int FontLib::fontSize() {
    return Session::instance()->getInteger("*font*size", 16);
}

// Initialize singleton_ to nullptr
FontLib* FontLib::singleton_ = nullptr;

/********************************************************************/
