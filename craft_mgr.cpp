#include "craft_mgr.h"
#include "logger.h"
#include "utility.h"
#include "translator.h"

#include <fstream>

using namespace std;

/********************************************************************/

Craft::Craft(const string& name, CraftType type, int time_in_seconds) : name_(name), type_(type), time_in_seconds_(time_in_seconds) {
}

void Craft::addCountedItem(const string& basic_item, int occurrences) {
    items_.push_back(CountedItem(BasicItem(basic_item), occurrences));
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

    // debug
    Craft* stone_furnace = new Craft("StoneFurnace", Craft::CraftType::OBJECT, 5);
    vector<Craft*> crafts;
    crafts.push_back(stone_furnace);
    crafts_by_machine_.insert( pair<string,vector<Craft*>>("WorkBench", crafts) );
    // end debug

    file.close();
}
