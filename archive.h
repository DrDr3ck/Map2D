#ifndef archive_h
#define archive_h

#include "map.h"
#include <string>

/********************************************************************/

class ArchiveConverter {
public:
    static std::string version();
    static bool load(GameBoard* board, const std::string& filename);
    static void save(GameBoard* board, const std::string& filename);
};

class DataConverter {
public:
    DataConverter() {}
    virtual ~DataConverter() {}

    virtual void load(const std::string& str) = 0;
    virtual void save(std::ofstream& file) = 0;
};

class MapDataConverter : public DataConverter {
public:
    MapDataConverter(MapData* data) : data_(data) {}
    virtual ~MapDataConverter() {}

    virtual void load(const std::string& str) override;
    virtual void save(std::ofstream& file) override;

protected:
    std::string typeTileToString(Tile::Type type) const;
    Tile::Type stringTileToType(const std::string& str) const;

    std::string btypeTileToString(Tile::BType type) const;
    Tile::BType stringTileToBType(const std::string& str) const;

    std::string ftypeTileToString(Tile::FType type) const;
    Tile::FType stringTileToFType(const std::string& str) const;

private:
    MapData* data_;
    int x_ = 0;
    int y_ = 0;
    int tile_id_;
    Tile::Type tile_type_ = Tile::BLOCK;
    int occurrences_;
    std::vector< std::pair<std::string,int> > items_;
    Tile::BType tile_btype_ = Tile::NONE;
    Tile::FType tile_ftype_ = Tile::METAL;
    bool inTile_ = false;
};

class CharacterConverter : public DataConverter {
public:
    CharacterConverter(PeopleGroup* group) : group_(group) {}
    virtual ~CharacterConverter() {}

    virtual void load(const std::string& str) override;
    virtual void save(std::ofstream& file) override;

private:
    PeopleGroup* group_;

    std::string name;
    Direction dir = {0,0};
    Position pos = {0,0};
    int activity_percentage = 0;
    int image_id = 0;
    bool inPeople = false;
    std::vector<BasicItem> carried_items;
};

#define NameValue std::pair<std::string,std::string>
#define Node std::pair<NameValue,std::vector<Attr>>

class ObjectConverter : public DataConverter {
public:
    ObjectConverter(MapData* data) : data_(data) {}
    virtual ~ObjectConverter() {}

    virtual void load(const std::string& str) override;
    virtual void save(std::ofstream& file) override;

protected:
    void save_nodes(std::ofstream& file, Object* object);

private:
    MapData* data_;

    std::string name;
    Position pos = {0,0};
    bool inObject = false;

    std::vector<Node> nodes;
};

class JobsConverter : public DataConverter {
public:
    JobsConverter(JobMgr* jobs_mgr) : jobs_mgr_(jobs_mgr) {}
    ~JobsConverter() = default;

    virtual void load(const std::string& str) override;
    virtual void save(std::ofstream& file) override;

private:
    JobMgr* jobs_mgr_;

    std::string name;
    std::string type;
    int build_time;
    Position pos = {0,0};
    int nb = 1;
    bool inJob = false;
};

/********************************************************************/

#endif // archive_h
