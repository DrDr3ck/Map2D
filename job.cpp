#include "job.h"
#include "sdl_camera.h"

/***********************************/

Job::Job(
    const std::string& name,
    Position tile_position,
    const std::string& icon_type, int build_time_ms
) : name_(name),
    tile_position_(tile_position),
    icon_type_(icon_type),
    build_time_ms_(build_time_ms),
    people_(nullptr)
{
}

void Job::render(SDLCamera* camera, const SDL_Rect& rect) {
    SDL_SetRenderDrawColor( camera->main_renderer(), 0, 250, 0, 255 );
    SDL_RenderFillRect( camera->main_renderer(), &rect );
}

/**************************************/

Job* JobMgr::getFirstAvailableJob() const {
    for( auto job : jobs_) {
        if( job->available() ) {
            return job;
        }
    }
    return nullptr;
}

bool JobMgr::findJobAt(Position tile_position) const {
    for( auto job : jobs_) {
        Position cur_position = job->tilePosition();
        if( cur_position.x == tile_position.x && cur_position.y == tile_position.y ) {
            return true;
        }
    }
    return false;
}

void JobMgr::addJob(Job* job) {
    if( !findJobAt(job->tilePosition()) ) {
        jobs_.push_back( job );
    }
}

void JobMgr::cancelJob(Position tile_position) {
    for( auto job : jobs_) {
        Position cur_position = job->tilePosition();
        if( cur_position.x == tile_position.x && cur_position.y == tile_position.y ) {
            jobs_.remove(job);
            return;
        }
    }
}

/**************************************/
