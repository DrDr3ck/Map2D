#ifndef craft_mgr_h
#define craft_mgr_h

#include <map>
#include <vector>
#include <string>

#include "items.h"

/********************************************************************/

class Craft {
public:
    enum class CraftType {ITEM, OBJECT};
public:
    Craft(const std::string& name, CraftType type);
    ~Craft() = default;

    const std::string& name() const { return name_; }
    void addCountedItem(const std::string& basic_item, int occurrences);
    const std::vector<CountedItem>& getItems() const { return items_; }
    CraftType type() const { return type_; }
    int time() const { return time_in_seconds_; }
    void setTime(int time_in_seconds) { time_in_seconds_ = time_in_seconds; }
    int occurrence() const { return occurrence_; }
    void setOccurrence(int occurrence) { occurrence_ = occurrence; }

private:
    std::string name_;
    CraftType type_;
    int time_in_seconds_ = 0;
    int occurrence_ = 0;
    std::vector<CountedItem> items_;
};

class CraftMgr {
private:
  CraftMgr() {}
  ~CraftMgr() {}

public:
    static CraftMgr* instance();
    static void kill();

    void loadCrafts(const std::string& filename);

    std::vector<Craft*> craftsForMachine(const std::string& machine);

    static std::string getPixmapName(Craft* craft);

private:
    static CraftMgr* singleton_;
    std::map<std::string, std::vector<Craft*> > crafts_by_machine_;
};

/********************************************************************/

#endif // craft_mgr_h
