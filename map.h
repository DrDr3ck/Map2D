#ifndef map_h
#define map_h

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

    int width() const { return width_; }
    int height() const { return height_; }
private:
    int width_;
    int height_;
    Tile* tiles_;
};

#endif // map_h
