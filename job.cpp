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

/**************************************/

JobMgr::JobMgr(SDL_Renderer* renderer) : renderer_(renderer) {
    if( renderer != nullptr ) {
        SDL_Surface* icon_surface = IMG_Load("none.png");
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, icon_surface);
        map_of_jobs_["none"] = texture;
        SDL_FreeSurface(icon_surface);
    }
}

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

SDL_Texture* JobMgr::getTexture(const std::string& icon_type) {
    if( map_of_jobs_.find(icon_type) != map_of_jobs_.end()) {
        return map_of_jobs_[icon_type];
    }
    // new icon: add it in the map
    std::string filename = icon_type;
    filename += ".png";
    SDL_Surface* icon_surface = IMG_Load(filename.c_str());
    if( icon_surface == nullptr ) {
        std::cout << "Cannot find icon for " << filename << std::endl;
        SDL_Texture* texture = map_of_jobs_["none"]; // return the 'none' icon texture
        map_of_jobs_[icon_type] = texture;
        return texture;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, icon_surface);
    map_of_jobs_[icon_type] = texture;
    SDL_FreeSurface(icon_surface);
    return texture;
}

void JobMgr::render(const Job& job, SDLCamera* camera, const SDL_Rect& rect) {
    SDL_Texture* texture = getTexture(job.iconType());
    camera->displayTexture(texture, &rect);
}


/**************************************/