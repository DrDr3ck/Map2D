#ifndef archive_h
#define archive_h

#include "map.h"
#include <string>

/********************************************************************/

class ArchiveConverter {
public:
    static void load(GameBoard* board, const std::string& filename);
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
    int x = 0;
    int y = 0;
    int tile_id;
    Tile::Type tile_type = Tile::BLOCK;
    Tile::BType tile_btype = Tile::NONE;
    Tile::FType tile_ftype = Tile::METAL;
    bool inTile = false;
};

class CharacterConverter : public DataConverter {
public:
    CharacterConverter(PeopleGroup* group) : group_(group) {}
    ~CharacterConverter() {}

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
};

class JobsConverter {
public:
    JobsConverter() {}
    ~JobsConverter() {}
};

/********************************************************************/

#endif // archive_h
