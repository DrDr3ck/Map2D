#include "craft_mgr.h"
#include "logger.h"
#include "utility.h"
#include "translator.h"

#include <fstream>
#include <algorithm>

using namespace std;

/********************************************************************/

Craft::Craft(const string& name, CraftType type) : name_(name), type_(type) {
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

namespace {
    std::string getName(const std::string& str) {
        int begin_index = str.find("name=");
        int end_index = str.find("\"", begin_index+6);
        std::string sub = str.substr(begin_index+6, end_index-begin_index-6);
        return sub;
    }

    int getOccurrence(const std::string& str, bool endtag=false) {
        int begin_index = str.find("occ=");
        if( begin_index < 0 ) return 1;
        int end_index = endtag ? str.find("/>") : str.find(">");
        std::string sub = str.substr(begin_index+4, end_index-begin_index-4);
        return atoi(sub.c_str());
    }
}

/*!
 * \return the pixmap name of the given \p craft
 */
std::string CraftMgr::getPixmapName(Craft* craft) {
    std::string craft_name;
    craft_name = "items/" + craft->name() + "_item.png";
    std::replace(craft_name.begin(), craft_name.end(), ' ', '_');
    return craft_name;
}

/*!
 * Loads crafts defined in \p filename
 */
void CraftMgr::loadCrafts(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        Logger::error() << tr("Cannot load craft, unable to open file: ") << filename << Logger::endl;
        return;
    }

    bool in_machine = false;
    bool in_object = false;
    bool in_finalItem = false;
    std::string machine_name;
    std::string object_name;
    std::string item_name;
    std::string end_machine_tag;
    std::string end_object_tag = "</object>";
    std::string end_item_tag = "</finalItem>";
    std::string str;
    Craft* craft = nullptr;
    vector<Craft*> crafts;
    while (std::getline(file, str)) {
        if( in_machine ) {
            str = Utility::trim(str, ' ');
            str = Utility::trim(str, '\t');
            if( in_object ) {
                if( Utility::startsWith(str,end_object_tag) ) {
                    in_object = false;
                    crafts.push_back(craft);
                } else if( Utility::startsWith(str, "<item") ) {
                    std::string name = getName(str);
                    int occ = getOccurrence(str, true);
                    craft->addCountedItem(name,occ);
                } else if( Utility::startsWith(str, "<time>") ) {
                    int end_index = str.find("</time>");
                    std::string time_str = str.substr(6,end_index-6);
                    int time = atoi(time_str.c_str());
                    craft->setTime(time);
                }
            } else if( in_finalItem ) {
                if( Utility::startsWith(str,end_item_tag) ) {
                    in_finalItem = false;
                    crafts.push_back(craft);
                } else if( Utility::startsWith(str, "<item") ) {
                    std::string name = getName(str);
                    int occ = getOccurrence(str, true);
                    craft->addCountedItem(name,occ);
                } else if( Utility::startsWith(str, "<time>") ) {
                    int end_index = str.find("</time>");
                    std::string time_str = str.substr(6,end_index-6);
                    int time = atoi(time_str.c_str());
                    craft->setTime(time);
                }
            } else {
                if( Utility::startsWith(str, "<object") ) {
                    object_name = getName(str);
                    in_object = true;
                    craft = new Craft(object_name, Craft::CraftType::OBJECT);
                    int occ = getOccurrence(str);
                    craft->setOccurrence(occ);
                } else if( Utility::startsWith(str, "<finalItem") ) {
                    std::string name = getName(str);
                    in_finalItem = true;
                    craft = new Craft(name, Craft::CraftType::ITEM);
                    int occ = getOccurrence(str);
                    craft->setOccurrence(occ);
                } else if( Utility::startsWith(str,end_machine_tag) ) {
                    in_machine = false;
                    std::transform(machine_name.begin(), machine_name.end(), machine_name.begin(), ::tolower);
                    crafts_by_machine_.insert( pair<string,vector<Craft*>>(machine_name, crafts) );
                    crafts.clear();
                }
            }
        } else {
            str = Utility::trim(str, ' ');
            if( str[0] == '<' && str[str.size()-1] == '>' ) {
                machine_name = str.substr(1,str.size()-2);
                in_machine = true;
                end_machine_tag = "</" + machine_name + ">";
            }
        }
    }

    file.close();
}

vector<Craft*> CraftMgr::craftsForMachine(const std::string& machine) {
    auto crafts_iterator = crafts_by_machine_.find(machine);
    if( crafts_iterator == crafts_by_machine_.end() ) {
        vector<Craft*> empty;
        return empty;
    }
    auto result = *crafts_iterator;
    return result.second;
}

Craft* CraftMgr::findCraft(const std::string& craft_name, const std::string& machine) const {
    auto crafts_iterator = crafts_by_machine_.find(machine);
    if( crafts_iterator == crafts_by_machine_.end() ) {
        Logger::debug() << "Machine " << machine << " does not exist" << Logger::endl;
        return nullptr;
    }
    auto result = *crafts_iterator;
    for( auto craft : result.second ) {
        if( craft->name() == craft_name ) {
            return craft;
        }
    }
    Logger::debug() << "Unable to find craft " << craft_name << " in machine " << machine << Logger::endl;
    return nullptr;
}
