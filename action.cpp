#include "action.h"
#include "character.h"
#include "job.h"
#include "path_finding.h"
#include "translator.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "SDL2/SDL_image.h"

/********************************************************************/

NoAction::NoAction(Character* character) : ActionBase(), character_(character) {
    max_time_spent_ = std::rand() % 5; // in seconds
    // TODO: random choice from list of no action descs
    if( max_time_spent_ > 2 )
        character->setAction(this, tr("Need a job"));
    else
        character->setAction(this, tr("Dreaming"));
}

bool NoAction::spentTime(double time_spent) {
    // no action: just do a random direction change around initial position
    if( time_spent/1000. > max_time_spent_ ) {
        character_->setDirection(Utility::randint(0,2)-1, Utility::randint(0,1));
        return false;
    }
    return true;
}

/********************************************************************/

MoveAction::MoveAction(Character* character, std::vector<Position>& path_in_tile, int tile_size) {
    character_ = character;
    origin_ = character_->tilePosition();
    if( !character_->validPixelPosition() ) {
        character_->setPixelPosition(origin_.x*tile_size, origin_.y*tile_size);
    }
    character_->setTimeSpent(0);
    create_animation_path(path_in_tile, tile_size);
    start_time_ = std::chrono::steady_clock::now();
    speed_ = 1;
    activity_percent_ = 0;
    destination_ = path_in_tile.at( path_in_tile.size()-1 );
    is_finished_ = false;
}

bool MoveAction::spentTime(double /*time_spent*/) {
    Position new_pixel_pos = next_position_;
    Position old_pixel_pos = prev_position_;
    if( new_pixel_pos.x == old_pixel_pos.x ) {
        character_->setDirection(0, character_->direction().y );
    } else if( new_pixel_pos.x < old_pixel_pos.x ) {
        character_->setDirection(-1, character_->direction().y );
    } else if( new_pixel_pos.x > old_pixel_pos.x ) {
        character_->setDirection(1, character_->direction().y );
    }
    if( new_pixel_pos.y == old_pixel_pos.y ) {
        character_->setDirection(character_->direction().x, 0 );
    } else if( new_pixel_pos.y < old_pixel_pos.y ) {
        character_->setDirection(character_->direction().x, 1 );
    } else if( new_pixel_pos.y > old_pixel_pos.y ) {
        character_->setDirection(character_->direction().x, -1 );
    }
    new_pixel_pos = get_position_in_pixel();
    character_->setPixelPosition(new_pixel_pos.x, new_pixel_pos.y);
    if( is_finished_ ) {
        return false;
    }
    return true;
}

void MoveAction::postAction() {
    character_->setTilePosition( destination_ );
    character_->setPixelPosition(-1,-1); // invalidate pixel position
    character_->setTimeSpent(0);
}

void MoveAction::create_animation_path(std::vector<Position>& path_in_tile, int tile_size) {
    path_in_pixel_.clear();
    path_in_tile.erase(path_in_tile.begin());
    if( character_->validPixelPosition() ) {
        path_in_pixel_.push_back( character_->pixelPosition() );
    }
    unsigned int i=0;
    for( auto position : path_in_tile ) {
        i++;
        Position cur = { position.x*tile_size, position.y*tile_size };
        if( i < path_in_tile.size() ) {
            // randomize a little bit by adding some pixels in the move
            cur.x = cur.x + Utility::randint(-2,8);
            cur.y = cur.y + Utility::randint(-2,8);
        }
        path_in_pixel_.push_back( cur );
    }
}

Position MoveAction::get_position_in_pixel() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    double cur_time = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_).count();
    cur_time *= speed_;
    cur_time = cur_time / 1000000.; // time spent in seconds
    activity_percent_ = int(100.0*cur_time/(path_in_pixel_.size()-1));
    if( cur_time >= (path_in_pixel_.size()-1) ) {
        is_finished_ = true;
        return path_in_pixel_.back();
    }
    int base = int(cur_time);
    Position prev = path_in_pixel_.at(base);
    int x1 = prev.x;
    int y1 = prev.y;
    Position cur_position = {
        int(std::round(x1/float(Utility::tileSize))),
        int(std::round(y1/float(Utility::tileSize))) };
    character_->setTilePosition( cur_position );
    prev_position_ = cur_position;
    Position next = path_in_pixel_.at(base+1);
    int x2 = next.x;
    int y2 = next.y;
    next_position_ = { int(std::round(x2/float(Utility::tileSize))), int(std::round(y2/float(Utility::tileSize)))  };
    double factor = cur_time - base;
    Position cur_in_pixel;
    cur_in_pixel.x = int(x1 + (x2-x1)*factor);
    cur_in_pixel.y = int(y1 + (y2-y1)*factor);
    return cur_in_pixel;
}

/********************************************************************/

BuildAction::BuildAction(
    GameBoard* game_board, Character* people, Job* job
) {
    game_board_ = game_board;
    people_ = people;
    job_ = job;
    action_ = nullptr;
    isValid_ = true;
}

BuildAction::~BuildAction() {
}

void BuildAction::preAction() {
    if( people_->tilePosition().x == job_->tilePosition().x && people_->tilePosition().y == job_->tilePosition().y ) {
        return;
    }
    PathFinding path(game_board_->data());
    Position end_position = job_->tilePosition();
    std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);

    if( positions.size() == 0 ) {
        isValid_ = false;
        job_->reset();
    } else {
        action_ = new MoveAction(people_, positions, Utility::tileSize);
    }
}

bool BuildAction::spentTime(double time_spent) {
    if( !isValid_ ) return false;
    if( action_ != nullptr ) {
        if( !action_->spentTime(time_spent) ) {
            action_->postAction();
            delete action_;
            action_ = nullptr;
            start_time_ = std::chrono::steady_clock::now();
            if( !game_board_->jobManager()->findJobAt(job_->tilePosition() ) ) {
                Logger::info() << tr("Job canceled") << Logger::endl;
                return false; // job has been canceled
            }
            // set direction so that people can 'work'
            people_->setDirection(people_->direction().x, 1);
        }
    } else {
        // spent time on construction...
        std::chrono::steady_clock::time_point cur_time = std::chrono::steady_clock::now();
        double delay_anim_us = std::chrono::duration_cast<std::chrono::microseconds>(cur_time - start_time_).count();
        double delta_time = delay_anim_us/1000.;
        people_->setActivityPercent( std::min(100,int(100.0*delta_time/job_->buildTime())) );
        if( delta_time > job_->buildTime() ) {
            return false;
        }
    }
    return true;
}

void BuildAction::postAction() {
    people_->setActivityPercent(0);
    if( !isValid_ ) return;
    Position position = job_->tilePosition();
    if( game_board_->jobManager()->findJobAt(position) ) {
        game_board_->jobManager()->cancelJob(position);
        if( job_->name() == DEMOLISHWALL ) {
            game_board_->data()->removeWall(position.x,position.y);
        } else if( job_->name() == BUILDWALL ) {
            game_board_->data()->addWall(position.x,position.y);
        } else if( job_->name() == BUILDFLOOR ) {
            game_board_->data()->addFloor(position.x,position.y);
        } else if( job_->name() == DEMOLISHFLOOR ) {
            game_board_->data()->removeFloor(position.x,position.y);
        } else if( job_->name() == BUILDOBJECT ) {
            BuildObjectJob* bjob = static_cast<BuildObjectJob*>(job_);
            Object* object = bjob->getObject();
            if( object != nullptr ) {
                game_board_->data()->addObject(object,position.x,position.y);
            } else {
                Logger::error() << "Cannot create object " << bjob->objectName() << Logger::endl;
            }
        }
    }
}

/********************************************************************/

ExtractAction::ExtractAction(
    GameBoard* game_board, Character* people, Job* job
) {
    game_board_ = game_board;
    people_ = people;
    job_ = job;
    action_ = nullptr;
    isValid_ = true;
}

ExtractAction::~ExtractAction() {
}

void ExtractAction::preAction() {
    if( people_->tilePosition().x == job_->tilePosition().x && people_->tilePosition().y == job_->tilePosition().y ) {
        return;
    }
    PathFinding path(game_board_->data());
    Position end_position = job_->tilePosition();
    std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);

    if( positions.size() == 0 ) {
        isValid_ = false;
        job_->reset();
    } else {
        action_ = new MoveAction(people_, positions, Utility::tileSize);
    }
}

bool ExtractAction::spentTime(double time_spent) {
    if( !isValid_ ) return false;
    if( action_ != nullptr ) {
        if( !action_->spentTime(time_spent) ) {
            action_->postAction();
            delete action_;
            action_ = nullptr;
            start_time_ = std::chrono::steady_clock::now();
            if( !game_board_->jobManager()->findJobAt(job_->tilePosition() ) ) {
                Logger::info() << tr("Job canceled") << Logger::endl;
                return false; // job has been canceled
            }
            // set direction so that people can 'work'
            people_->setDirection(people_->direction().x, 1);
        }
    } else {
        // spent time on extraction...
        std::chrono::steady_clock::time_point cur_time = std::chrono::steady_clock::now();
        double delay_anim_us = std::chrono::duration_cast<std::chrono::microseconds>(cur_time - start_time_).count();
        double delta_time = delay_anim_us/1000.;
        people_->setActivityPercent( std::min(100,int(100.0*delta_time/job_->buildTime())) );
        if( delta_time > job_->buildTime() ) {
            if( job_->isRepetitive() ) {
                // repeat action
                if( job_->name() == EXTRACT ) {
                    Position position = job_->tilePosition();
                    game_board_->data()->extractItemFromTile(position.x,position.y);
                }
                start_time_ = std::chrono::steady_clock::now();
                return true;
            }
            return false;
        }
    }
    return true;
}

void ExtractAction::postAction() {
    people_->setActivityPercent(0);
    if( !isValid_ ) return;
    Position position = job_->tilePosition();
    if( game_board_->jobManager()->findJobAt(position) ) {
        game_board_->jobManager()->cancelJob(position);
        if( job_->name() == EXTRACT ) {
            game_board_->data()->extractItemFromTile(position.x,position.y);
        }
    }
}

/********************************************************************/

// Clean the Tile by transporting items in a chest

CleanAction::CleanAction(
    GameBoard* game_board, Character* people, Job* job
) {
    game_board_ = game_board;
    people_ = people;
    job_ = job;
    action_ = nullptr;
    isValid_ = true;
}

CleanAction::~CleanAction() {
}

void CleanAction::preAction() {
    // first check if robot can carry or not
    int max_carriable = people_->maxCarriable();
    Tile& cur =game_board_->data()->tile(job_->tilePosition().x,job_->tilePosition().y);
    if( max_carriable > 0 && cur.counted_items().size() > 0 ) {
        // go to the tile if still not the case
        if( people_->tilePosition().x == job_->tilePosition().x && people_->tilePosition().y == job_->tilePosition().y ) { // robot is over the tile: transfer items to robot
            // take all what you can by transferring max items from Tile to People
            if( !game_board_->data()->transferItems(people_) ) {
                // no action, Tile is empty
                return; // end of cleaning action
            }
            // then move to the nearest non full chest
            Object* object = game_board_->data()->getNearestChest(people_->tilePosition());
            PathFinding path(game_board_->data());
            Position end_position = object->tilePosition();
            std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);
            if( positions.size() == 0 ) {
                // cannot achieve the job
                isValid_ = false;
                job_->reset();
            } else {
                action_ = new MoveAction(people_, positions, Utility::tileSize);
            }
        } else { // move robot to tile
            PathFinding path(game_board_->data());
            Position end_position = job_->tilePosition();
            std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);
            if( positions.size() == 0 ) {
                // cannot achieve the job
                isValid_ = false;
                job_->reset();
            } else {
                action_ = new MoveAction(people_, positions, Utility::tileSize);
            }
        }
    } else {
        // robot is full
        Object* object = game_board_->data()->getNearestChest(people_->tilePosition());
        if( people_->tilePosition().x == object->tilePosition().x && people_->tilePosition().y == object->tilePosition().y ) { // robot is over the chest: drop items
            Chest* chest = static_cast<Chest*>(object);
            game_board_->data()->transferItems(people_, chest); // transfer all items from robot to chest
            // then move to the tile, again !
            PathFinding path(game_board_->data());
            Position end_position = job_->tilePosition();
            std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);
            if( positions.size() == 0 ) {
                // cannot achieve the job
                isValid_ = false;
                job_->reset();
            } else {
                if( cur.counted_items().size() > 0 ) {
                    action_ = new MoveAction(people_, positions, Utility::tileSize);
                } // otherwise, end of cleaning action
            }
        } else { // move robot over the chest
            PathFinding path(game_board_->data());
            Position end_position = object->tilePosition();
            std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);
            if( positions.size() == 0 ) {
                // cannot achieve the job
                isValid_ = false;
                job_->reset();
            } else {
                action_ = new MoveAction(people_, positions, Utility::tileSize);
            }
        }
    }
}

bool CleanAction::spentTime(double time_spent) {
    if( !isValid_ ) return false;
    if( action_ != nullptr ) {
        if( !action_->spentTime(time_spent) ) {
            action_->postAction();
            delete action_;
            action_ = nullptr;
            start_time_ = std::chrono::steady_clock::now();
            if( !game_board_->jobManager()->findJobAt(job_->tilePosition() ) ) {
                Logger::info() << tr("Job canceled") << Logger::endl;
                return false; // job has been canceled
            }
            // set direction so that people can 'work'
            people_->setDirection(people_->direction().x, 1);
            preAction();
        }
    } else {
        return false; // end of cleaning action
    }
    return true;
}

void CleanAction::postAction() {
    people_->setActivityPercent(0);
    if( !isValid_ ) return;
    Position position = job_->tilePosition();
    if( game_board_->jobManager()->findJobAt(position) ) {
        game_board_->jobManager()->cancelJob(position);
    }
}
