#ifndef map_h
#define map_h

#include "chest.h"

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

class Tile {
public:
    enum Type { BLOCK, DOOR, FLOOR, EMPTY, WALL };
    enum BType { NONE, GRASS, DIRT, ROCK };
    enum FType { METAL, PLASTIC };
public:
    Tile(int id=0, Type type=EMPTY, BType background_type=NONE, FType floor_type=METAL);
    ~Tile();

    int id() const;
    Type type() const;
    BType background_type() const;
    FType floor_type() const;
    void setTile(int id, Type type, BType background_type, FType floor_type);

    static std::string typeTileToString(Tile::Type type);
private:
    int id_;
    Type type_;
    BType background_type_;
    FType floor_type_;
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

    void addGround(int x, int y);
    void removeGround(int x, int y);

    void addObject(Object* object, int x, int y);
    std::vector<PositionObject>& objects() { return objects_; }

    void reset(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }
private:
    int width_;
    int height_;
    Tile* tiles_;
    std::vector<PositionObject> objects_;
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
    SDL_Surface* grounds() { return grounds_surface_; }
    std::map<int, SDL_Texture*>& mapOfTiles() { return mapOfTiles_; }
    std::map<int, SDL_Texture*>& mapOfWalls() { return mapOfWalls_; }
    std::map<int, SDL_Texture*>& mapOfGrounds() { return mapOfGrounds_; }
private:
    static TileSetLib* singleton_;
    SDL_Surface* tiles_surface_;
    SDL_Surface* walls_surface_;
    SDL_Surface* grounds_surface_;
    std::map<int, SDL_Texture*> mapOfTiles_;
    std::map<int, SDL_Texture*> mapOfWalls_;
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
