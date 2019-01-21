#ifndef craft_mgr_h
#define craft_mgr_h

#include <map>
#include <vector>
#include <string>

#include "items.h"

/********************************************************************/

class Craft {
public:
    Craft(const std::string& name);
    ~Craft();

    const std::string& name() const { return name_; }
private:
    std::string name_;
    std::vector<CountedItem> items_;
    int time_in_seconds_;

};

class CraftMgr {
private:
  CraftMgr() {}
  ~CraftMgr() {}

public:
    static CraftMgr* instance();
    static void kill();

    void loadCrafts(const std::string& filename);

private:
    static CraftMgr* singleton_;
    std::map<std::string, std::vector<Craft*> > crafts_by_machine_;
};

/********************************************************************/

#endif // craft_mgr_h
