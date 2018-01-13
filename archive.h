#ifndef archive_h
#define archive_h

#include "map.h"
#include <string>

/********************************************************************/

class MapDataConverter {
public:
    MapDataConverter() {}
    ~MapDataConverter() {}

    void load(MapData* data, const std::string& filename);
    void save(MapData* data, const std::string& filename);

protected:
    std::string typeTileToString(Tile::Type type) const;
    Tile::Type stringTileToType(const std::string& str) const;
};

/********************************************************************/

#endif // archive_h
