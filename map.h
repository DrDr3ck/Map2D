#ifndef map_h
#define map_h

#include <SDL2/SDL.h>
#include <map>

class Tile {
public:
    enum Type { BLOCK, DOOR, EMPTY };
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

class TileManager {
private:
  TileManager();
  ~TileManager() { }

public:
    static TileManager* instance();
    static void kill();

    static SDL_Texture* getTextureFromTileId(int id, SDL_Renderer* renderer);

    SDL_Surface* tiles() { return tiles_surface_; }
    std::map<int, SDL_Texture*>& mapOfTiles() { return mapOfTiles_; }
private:
    static TileManager* singleton_;
    SDL_Surface* tiles_surface_;
    std::map<int, SDL_Texture*> mapOfTiles_;
};

#endif // map_h
