#ifndef map_h
#define map_h

#include <SDL2/SDL.h>
#include <map>

/********************************************************************/

class Utility {
public:
    static std::string itos(int i);
};

/********************************************************************/

class Item {
public:
    Item() {}
    virtual ~Item() {}

protected:
};

/********************************************************************/

class Tile {
public:
    enum Type { BLOCK, DOOR, EMPTY, WALL };
    enum BType { NONE, GRASS, EARTH, ROCK };
public:
    Tile(int id=0, Type type=EMPTY, BType background_type=NONE);
    ~Tile();

    int id() const;
    Type type() const;
    BType background_type() const;
    void setTile(int id, Type type, BType background_type);

    static std::string typeTileToString(Tile::Type type);
private:
    int id_;
    Type type_;
    BType background_type_;
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

    void reset(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }
private:
    int width_;
    int height_;
    Tile* tiles_;
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

#endif // map_h
