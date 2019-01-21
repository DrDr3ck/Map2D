#include "craft_mgr.h"
#include "logger.h"
#include "utility.h"
#include "translator.h"

#include <fstream>

/********************************************************************/

Craft::Craft(const std::string& name) : name_(name) {
}

/********************************************************************/

// Initialize singleton_ to nullptr
CraftMgr* CraftMgr::singleton_ = nullptr;

CraftMgr* CraftMgr::instance() {
    if( singleton_ == nullptr ) {
        singleton_ = new CraftMgr();
        Logger::debug() << "creating CraftMgr singleton" << Logger::endl;
    }
    return singleton_;
}

void CraftMgr::kill() {
    if( singleton_ != nullptr ) {
        Logger::debug() << "destroying CraftMgr singleton" << Logger::endl;
        delete singleton_;
        singleton_ = nullptr;
    }
}

void CraftMgr::loadCrafts(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        Logger::error() << tr("Cannot load craft, unable to open file: ") << filename << Logger::endl;
        return;
    }

    std::string str;
    while (std::getline(file, str)) {
    }

    file.close();
}
