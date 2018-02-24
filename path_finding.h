#ifndef path_finding_h
#define path_finding_h

#include "map.h"
#include <vector>

/********************************************************************/

class PathFinding {
public:
  PathFinding(MapData* mapdata);
  ~PathFinding();

  std::vector<Position> findPath(Position start, Position end);

protected:
    void resetPath();
    int getIndex(int x, int y) const;
    std::vector<Position> list_actual_neighbours(int x, int y);
    Position get_prev_pos(Position position, int step);
    bool findWithStep(int step);

private:
    bool found_position_ = false;
    Position found_;
    MapData* map_data_;
    int* map_array_ = nullptr;
    int width_;
    int height_;
};

/********************************************************************/

#endif // path_finding_h
