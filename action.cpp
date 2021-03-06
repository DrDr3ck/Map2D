#include "action.h"
#include "character.h"
#include "job.h"
#include "path_finding.h"
#include "sdl_camera.h"
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
    cur_time_ = 0;
    speed_ = 1;
    activity_percent_ = 0;
    destination_ = path_in_tile.at( path_in_tile.size()-1 );
    is_finished_ = false;
}

bool MoveAction::spentTime(double time_spent) {
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
    cur_time_ = time_spent*1000;
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
    double cur_time = cur_time_;
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

JobActionBase::JobActionBase(GameBoard* game_board, Character* people, Job* job) {
    game_board_ = game_board;
    people_ = people;
    job_ = job;
}

JobActionBase::~JobActionBase() {
}

void JobActionBase::cancelAction() {
    if( job_ != nullptr ) {
        job_->takeJob(nullptr);
    }
    if( people_ != nullptr ) {
        people_->setActivityPercent(0);
    }
}

/********************************************************************/

BuildAction::BuildAction(
    GameBoard* game_board, Character* people, Job* job
) : JobActionBase(game_board, people, job) {
    action_ = nullptr;
}

BuildAction::~BuildAction() {
}

void BuildAction::reset() {
    if( job_ != nullptr ) {
        job_->reset();
    }
    is_valid_ = false;
}

void BuildAction::preAction() {
    if( people_->tilePosition().x == job_->tilePosition().x && people_->tilePosition().y == job_->tilePosition().y ) {
        return;
    }
    PathFinding path(game_board_->data());
    Position end_position = job_->tilePosition();
    std::vector<Position> positions = path.findPath(people_->tilePosition(), end_position);

    if( positions.size() == 0 ) {
        reset();
    } else {
        action_ = new MoveAction(people_, positions, Utility::tileSize);
    }
}

bool BuildAction::spentTime(double time_spent) {
    if( !is_valid_ ) return false;
    if( action_ != nullptr ) {
        if( !action_->spentTime(time_spent) ) {
            action_->postAction();
            delete action_;
            action_ = nullptr;
            if( !game_board_->jobManager()->findJobAt(job_->tilePosition() ) ) {
                Logger::info() << tr("Job canceled") << Logger::endl;
                return false; // job has been canceled
            }
            // set direction so that people can 'work'
            people_->setDirection(people_->direction().x, 1);
        }
    } else {
        // spent time on construction...
        double delta_time = time_spent;
        people_->setActivityPercent( std::min(100,int(100.0*delta_time/job_->buildTime())) );
        if( delta_time > job_->buildTime() ) {
            return false;
        }
    }
    return true;
}

void BuildAction::postAction() {
    people_->setActivityPercent(0);
    Position position = job_->tilePosition();
    if( game_board_->jobManager()->findJobAt(position) ) {
        game_board_->jobManager()->cancelJob(position);
        if( !is_valid_ ) return;
        MapData* data = game_board_->data();
        if( job_->name() == BUILDWALL ) {
            if( data->removeItemFromChest(position, BasicItem("wall")) ) {
                data->addWall(position.x,position.y);
            }
        } else if( job_->name() == BUILDFLOOR ) {
            if( data->removeItemFromChest(position, BasicItem("floor")) ) {
                data->addFloor(position.x,position.y);
            }
        } else if( job_->name() == BUILDFIELD ) {
            data->addField(position.x,position.y);
        } else if( job_->name() == DEMOLISHFOUNDATION ) {
            // demolish WALL or DOOR or FLOOR
            const Tile& cur_tile = data->tile(position.x, position.y);
            if( Tile::isWall(cur_tile) ) {
                data->removeWall(position.x,position.y);
                // put item in associated chest or put it on the floor
                data->store(BasicItem("wall"), position);
            } else if( Tile::isDoor(cur_tile) ) {
                data->removeDoor(position.x,position.y);
                // put item in associated chest or put it on the floor
                data->store(BasicItem("door"), position);
            } else if( Tile::isFloor(cur_tile) ) {
                data->removeFloor(position.x,position.y);
                // put item in associated chest or put it on the floor
                data->store(BasicItem("floor"), position);
            }
        } else if( job_->name() == BUILDDOOR ) {
            if( data->removeItemFromChest(position, BasicItem("door")) ) {
                data->addDoor(position.x,position.y);
            }
        } else if( job_->name() == BUILDOBJECT ) {
            BuildObjectJob* bjob = static_cast<BuildObjectJob*>(job_);
            Object* object = bjob->getObject();
            if( object != nullptr ) {
                if( data->removeItemFromChest(position, BasicItem(object->name())) ) {
                    data->addObject(object,position.x,position.y);
                } else {
                    std::string error = tr("Cannot remove object $1 from chest");
                    Utility::replace(error, "$1", object->name());
                    Logger::error() << error << Logger::endl;
                }
            } else {
                std::string error = tr("Cannot create object $1");
                Utility::replace(error, "$1", bjob->objectName());
                Logger::error() << error << Logger::endl;
            }
        } else if( job_->name() == DEMOLISHOBJECT ) {
            Object* object = data->getObject(position);
            if( object != nullptr ) {
                data->removeObject(position.x, position.y);
            } else {
                Logger::error() << tr("No object on this tile") << Logger::endl;
            }
        }
    }
}

/********************************************************************/

ExtractAction::ExtractAction(
    GameBoard* game_board, Character* people, Job* job
) : JobActionBase(game_board, people, job) {
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
            if( !game_board_->jobManager()->findJobAt(job_->tilePosition() ) ) {
                Logger::info() << tr("Job canceled") << Logger::endl;
                return false; // job has been canceled
            }
            // set direction so that people can 'work'
            people_->setDirection(people_->direction().x, 1);
        }
    } else {
        // spent time on extraction...
        double delta_time = time_spent;
        people_->setActivityPercent( std::min(100,int(100.0*delta_time/job_->buildTime())) );
        if( delta_time > job_->buildTime() ) {
            if( job_->isRepetitive() ) {
                job_->repeat();
                // repeat action
                if( job_->name() == EXTRACT ) {
                    Position position = job_->tilePosition();
                    game_board_->data()->extractItemFromTile(position.x,position.y);
                }
                people_->setTimeSpent(0); // reset time for next item
                return true;
            }
            return false;
        }
    }
    return true;
}

void ExtractAction::postAction() {
    people_->setActivityPercent(0);
    Position position = job_->tilePosition();
    if( game_board_->jobManager()->findJobAt(position) ) {
        game_board_->jobManager()->cancelJob(position);
        if( !isValid_ ) return;
        if( job_->name() == EXTRACT ) {
            game_board_->data()->extractItemFromTile(position.x,position.y);
        }
    }
}

/********************************************************************/

// Clean the Tile by transporting items in a chest

CleanAction::CleanAction(
    GameBoard* game_board, Character* people, Job* job
) : JobActionBase(game_board, people, job) {
    action_ = nullptr;
    isValid_ = true;
}

CleanAction::~CleanAction() {
}

void CleanAction::preAction() {
    // first check if robot can carry or not
    Tile& cur =game_board_->data()->tile(job_->tilePosition().x,job_->tilePosition().y);
    if( people_->maxCarriable() > 0 && cur.counted_items().size() > 0 ) {
        // go to the tile if still not the case
        if( people_->tilePosition().x == job_->tilePosition().x && people_->tilePosition().y == job_->tilePosition().y ) { // robot is over the tile: transfer items to robot
            // take all what you can by transferring max items from Tile to People
            if( !game_board_->data()->transferItems(people_) ) {
                // no action, Tile is empty
                return; // end of cleaning action
            }
            // then move to the nearest non full chest
            Object* object = game_board_->data()->getNearestEmptyChest(people_->tilePosition(), people_->carriedItems().at(0));
            if( object == nullptr ) {
                Logger::warning() << tr("Not enough space in chests, please create a new one") << Logger::endl;
                isValid_ = false;
                job_->reset();
                return;
            }
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
        // robot is full, need to move to nearest none full chest
        if( people_->carriedItems().size() == 0 ) {
            Logger::error() << tr("Robot is not full") << Logger::endl;
            isValid_ = false;
            job_->reset();
            return;
        }
        Object* object = game_board_->data()->getNearestEmptyChest(people_->tilePosition(), people_->carriedItems().at(0));
        if( object == nullptr ) {
            Logger::warning() << tr("Not enough space in chests, please create a new one") << Logger::endl;
            isValid_ = false;
            job_->reset();
            return;
        }
        if( people_->tilePosition().x == object->tilePosition().x && people_->tilePosition().y == object->tilePosition().y ) { // robot is over the chest: drop items
            Chest* chest = static_cast<Chest*>(object);
            game_board_->data()->transferItems(people_, chest); // transfer all items from robot to chest
            // check if robot has still some items to transfer in another chest
            if( people_->carriedItems().size() > 0 ) {
                Object* object = game_board_->data()->getNearestEmptyChest(people_->tilePosition(), people_->carriedItems().at(0));
                if( object == nullptr ) {
                    Logger::warning() << tr("Not enough space in chests, please create a new one") << Logger::endl;
                    isValid_ = false;
                    job_->reset();
                    return;
                }
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
            } else {
                // or move to the tile, again !
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
    Position position = job_->tilePosition();
    if( game_board_->jobManager()->findJobAt(position) ) {
        game_board_->jobManager()->cancelJob(position);
        if( !isValid_ ) return;
    }
}
