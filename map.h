#ifndef map_h
#define map_h

#include <SDL2/SDL.h>
#include <map>

/********************************************************************/

class Tile {
public:
    enum Type { BLOCK, DOOR, EMPTY, WALL };
public:
    Tile(int id=0, Type type=EMPTY);
    ~Tile();

    int id() const;
    Type type() const;
    void setTile(int id, Type type);
private:
    int id_;
    Type type_;
};

/********************************************************************/

class MapData {
public:
    MapData(int width, int height);
    ~MapData();

    const Tile& tile(int x,int y) const;
    Tile& tile(int x,int y);

    void reset(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }
private:
    int width_;
    int height_;
    Tile* tiles_;
};

/********************************************************************/

class TileSet {
private:
  TileSet();
  ~TileSet();

public:
    static TileSet* instance();
    static void kill();

    static SDL_Texture* getTextureFromTile(const Tile& tile, SDL_Renderer* renderer);

    SDL_Surface* tiles() { return tiles_surface_; }
    SDL_Surface* walls() { return walls_surface_; }
    std::map<int, SDL_Texture*>& mapOfTiles() { return mapOfTiles_; }
    std::map<int, SDL_Texture*>& mapOfWalls() { return mapOfWalls_; }
private:
    static TileSet* singleton_;
    SDL_Surface* tiles_surface_;
    SDL_Surface* walls_surface_;
    std::map<int, SDL_Texture*> mapOfTiles_;
    std::map<int, SDL_Texture*> mapOfWalls_;
};

/********************************************************************/

#endif // map_h
