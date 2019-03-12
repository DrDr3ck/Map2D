#ifndef map_h
#define map_h

#include "object.h"
#include "character.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <map>
#include <cstdlib>
#include <iostream>
#include <sstream>

class JobMgr;

/********************************************************************/

struct TerrainType {
    int red;
    int green;
    int blue;
    int height;
    std::string label;
};

class MapUtility {
public:
    static TerrainType* readColorMap(const std::string& filename);
    static bool saveColorMap(const std::string& filename, TerrainType* regions, int regionCount);
};

struct TileItem {
    char name; // B for background, S for sol, M for mur, P for porte and F for fenetre
    int type; // for example: 0 for water, 1 for sand, 2 for grass, ...
    int value; // pixmap item
};

class Tile {
public:
    enum Type { BLOCK, DOOR, FLOOR, EMPTY, WALL };
    enum BType { NONE, WATER, SAND, DIRT, GRASS, ROCK, COAL, COPPER, IRON };
    enum FType { METAL, PLASTIC };
public:
    Tile(int id=0, Type cell_type=EMPTY, BType background_type=NONE, FType floor_type=METAL);
    ~Tile();

    int id() const;
    Type cell_type() const;
    BType background_type() const;
    FType floor_type() const;
    int occurrences() const;
    const std::vector<CountedItem>& counted_items() const { return counted_items_; }
    std::vector<CountedItem>& counted_items() { return counted_items_; }

    void setTile(int id, Type cell_type, BType background_type, FType floor_type);
    void setCellTile(int id, Type cell_type);
    void setBackgroundTile(int id, BType background_type);
    void setFloorTile(int id, FType floor_type);

    void setOccurrences(int value);

    static std::string typeTileToString(Tile::Type type);
    static std::string btypeTileToString(Tile::BType type);

    static bool isWall(const Tile& tile);
    static bool isFloor(const Tile& tile);
    static bool isDoor(const Tile& tile);

    void addItem(const BasicItem& item, int nb=1);
    std::string removeItem(const BasicItem& item, int nb=1);
private:
    int id_;
    Type cell_type_;
    int occurrences_;
    BType background_type_;
    FType floor_type_;
    std::vector<CountedItem> counted_items_;
};

/********************************************************************/

class MapData {
public:
    MapData(int width, int height);
    ~MapData();

    const Tile& tile(int x,int y) const;
    Tile& tile(int x,int y);
    void addWall(int x, int y);
    void removeWall(int x, int y);

    void addFloor(int x, int y);
    void removeFloor(int x, int y);

    void addDoor(int x, int y);
    void removeDoor(int x, int y);

    void addGround(int x, int y);
    void removeGround(int x, int y);

    void extractItemFromTile(int x,int y);
    void cleanItemFromTile(int x,int y,Character* people);

    void addObject(Object* object, int x, int y);
    bool removeObject(int x, int y);
    Object* getObject(Position tile_position);
    bool store(const BasicItem& item, Position tile_position, int occurrence = 1);
    std::vector<Object*>& objects() { return objects_; }

    void reset(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }

    void setMapImageName(const std::string& map_image_name);
    const std::string& getMapImageName() const;

    bool transferItems(Character* people);
    void transferItems(Character* people, Chest* chest);
    Object* getNearestChest(Position position);
    bool removeItemFromChest(Position position, const BasicItem& item);
    Object* getNearestEmptyChest(Position position, const BasicItem& item);
    Object* getAssociatedChest(Position position);

    static void createMap(MapData* data);
private:
    int width_;
    int height_;
    std::string map_image_name_;
    Tile* tiles_;
    std::vector<Object*> objects_;
};

/********************************************************************/

class TileSetLib {
private:
  TileSetLib();
  ~TileSetLib();

public:
    static TileSetLib* instance();
    static void kill();

    static SDL_Texture* getTextureFromTile(const Tile& tile, SDL_Renderer* renderer);

    SDL_Surface* tiles() { return tiles_surface_; }
    SDL_Surface* walls() { return walls_surface_; }
    SDL_Surface* doors() { return doors_surface_; }
    SDL_Surface* grounds() { return grounds_surface_; }
    std::map<int, SDL_Texture*>& mapOfTiles() { return mapOfTiles_; }
    std::map<int, SDL_Texture*>& mapOfWalls() { return mapOfWalls_; }
    std::map<int, SDL_Texture*>& mapOfDoors() { return mapOfDoors_; }
    std::map<int, SDL_Texture*>& mapOfGrounds() { return mapOfGrounds_; }
private:
    static TileSetLib* singleton_;
    SDL_Surface* tiles_surface_;
    SDL_Surface* walls_surface_;
    SDL_Surface* doors_surface_;
    SDL_Surface* grounds_surface_;
    std::map<int, SDL_Texture*> mapOfTiles_;
    std::map<int, SDL_Texture*> mapOfWalls_;
    std::map<int, SDL_Texture*> mapOfDoors_;
    std::map<int, SDL_Texture*> mapOfGrounds_;
};

/********************************************************************/

class PeopleGroup;

class GameBoard {
public:
    GameBoard(PeopleGroup* group, MapData* data, JobMgr* manager);
    ~GameBoard();

    void animate(double delay_ms);

    MapData* data() const { return data_; }
    PeopleGroup* group() const { return group_; }
    JobMgr* jobManager() const { return job_mgr_; }

protected:
    PeopleGroup* group_;
    MapData* data_;
    JobMgr* job_mgr_;
};

/********************************************************************/

#endif // map_h
