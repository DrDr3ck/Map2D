#ifndef job_h
#define job_h

#include "map.h"
#include "character.h"
#include <string>
#include <list>

/********************************************************************/

class Job {
public:
    Job(const std::string& name, Position tile_position, const std::string& icon_type, int build_time_ms);
    virtual ~Job() {}

    const std::string& name() const { return name_; }
    Position tilePosition() const { return tile_position_; }
    const std::string& iconType() const { return icon_type_; }
    int build_time() const { return build_time_ms_; }
    Character* people() const { return people_; }

    void takeJob(Character* people) {
        people_ = people;
    }

    bool available() const {
        return people_ == nullptr;
    }

    virtual void render(SDLCamera* camera, const SDL_Rect& rect);

private:
    std::string name_;
    Position tile_position_;
    std::string icon_type_;
    int build_time_ms_;

    // if nullptr, job is available otherwise someone plans to work on it
    Character* people_ = nullptr;
};

/********************************************************************/

class BuildJob : public Job {
public:
    BuildJob(
        Position tile_position, const std::string& icon_type, int build_time_ms
    ) : Job("build", tile_position, icon_type, build_time_ms)
    {
    }
};

/********************************************************************/

class JobMgr {
public:
    JobMgr() {}
    ~JobMgr() {}

    int size() const {
        return jobs_.size();
    }
    Job* getFirstAvailableJob() const;
    bool findJobAt(Position tile_position) const;
    void addJob(Job* job);
    void cancelJob(Position tile_position);

    const std::list<Job*>& jobs() const {
        return jobs_;
    }

private:
    std::list<Job*> jobs_;
};

/********************************************************************/

#endif // job_h
