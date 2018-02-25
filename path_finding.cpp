#include "path_finding.h"

/********************************************************************/

PathFinding::PathFinding(MapData* mapdata) {
    map_data_ = mapdata;
    width_ = map_data_->width();
    height_ = map_data_->height();
}

PathFinding::~PathFinding() {
    map_data_ = nullptr;
}

int PathFinding::getIndex(int x, int y) const {
    return x + y*width_;
}

void PathFinding::resetPath() {
    delete[] map_array_;
    found_position_ = false;
    map_array_ = new int[width_*height_];
    for( int i = 0; i < width_; i++ ) {
        for( int j = 0; j < height_; j++ ) {
            int index = getIndex(i,j);
            map_array_[index] = 0;
        }
    }
}

std::vector<Position> PathFinding::findPath(Position start, Position end) {
    // create empty map
    resetPath();
    // initialize start and end
    map_array_[getIndex(start.x, start.y)] = 1;
    map_array_[getIndex(end.x, end.y)] = -1;

    int cur_step = 1;
    bool has_new_step = findWithStep(cur_step);
    while( !found_position_ && has_new_step ) {
        cur_step += 1;
        has_new_step = findWithStep(cur_step);
    }

    std::vector<Position> build_positions;
    // no way to reach destination. returns empty vector.
    if( !found_position_ ) {
        return build_positions;
    }
    //dump();

    build_positions.push_back( found_ );
    for( int i = cur_step; i > 0; i-- ) {
        build_positions.push_back( get_prev_pos(build_positions.back(), i) );
    }
    // reverse
    std::vector<Position>::reverse_iterator rit = build_positions.rbegin();
    std::vector<Position> positions;
    for (; rit!= build_positions.rend(); ++rit) {
        Position position = *rit;
        positions.push_back(position);
    }

    return positions;
}

Position PathFinding::get_prev_pos(Position position, int step) {
    std::vector<Position> neighbours = list_actual_neighbours(position.x, position.y);
    for( auto cur_position : neighbours ) {
        int index = getIndex(cur_position.x, cur_position.y );
        if( map_array_[index] == step ) {
            return cur_position;
        }
    }
    Position error = {-1,-1};
    return error;
}

/*!
 * \return the list of neighbours for a given position, cells that are behind a wall is not added
 */
std::vector<Position> PathFinding::list_actual_neighbours(int x, int y) {
    std::vector<Position> result;

    if( x > 0 ) {
        Position position = {x-1,y};
        if( map_data_->tile(position.x,position.y).type() != Tile::WALL ) {
            result.push_back(position);
        }
    }
    if( x < width_-1 ) {
        Position position = {x+1,y};
        if( map_data_->tile(position.x,position.y).type() != Tile::WALL ) {
            result.push_back(position);
        }
    }
    if( y > 0 ) {
        Position position = {x,y-1};
        if( map_data_->tile(position.x,position.y).type() != Tile::WALL ) {
            result.push_back(position);
        }
    }
    if( y < height_-1 ) {
        Position position = {x,y+1};
        if( map_data_->tile(position.x,position.y).type() != Tile::WALL ) {
            result.push_back(position);
        }
    }

    //if True and self.walls is not None:
    //    new_res = list()
    //    for r in result:
    //        if r in self.start_positions or r in self.final_positions or r not in self.walls:
    //            new_res.append(r)
    //    result = new_res
    //random.shuffle(result)

    int n = result.size();
    if( n == 0 ) {
        return result;
    }
    for( int i = n-1; i > 0; i-- ) {
        int ni = Utility::randint(0,i);
        if( ni == i ) {
            continue;
        }
        Position pos = result[i];
        result[i] = result[ni];
        result[ni] = pos;
    }
    return result;
}

bool PathFinding::findWithStep(int cur_step) {
    bool has_new_step = false;
    for( int x = 0; x < width_; x++ ) {
        for( int y = 0; y < height_; y++ ) {
            int index = getIndex(x,y);
            if( map_array_[index] == cur_step ) {
                // check around
                std::vector<Position> neighbours = list_actual_neighbours(x,y);
                for( auto position : neighbours ) {
                    int nx = position.x;
                    int ny = position.y;
                    int index = getIndex(nx,ny);
                    if( map_array_[index] > 0 ) {
                        continue; // do not overwrite previous value from other start
                    }
                    if( map_array_[index] == -1 ) {
                        found_position_ = true;
                        found_ = {nx,ny};
                        break;
                    }
                    map_array_[index] = cur_step+1;
                    has_new_step = true;
                }
            }
        }
    }
    return has_new_step;
}

/********************************************************************/
