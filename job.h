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
    int buildTime() const { return build_time_ms_; }
    void setBuildTime(int build_time_in_ms) { build_time_ms_ = build_time_in_ms; }
    Character* people() const { return people_; }

    virtual bool isRepetitive() { return false; }
    virtual void repeat() {}

    void reset() {
        people_ = nullptr;
    }

    void takeJob(Character* people) {
        people_ = people;
    }

    bool available() const {
        return people_ == nullptr;
    }

private:
    std::string name_;
    Position tile_position_;
    std::string icon_type_;
    int build_time_ms_;

    // if nullptr, job is available otherwise someone plans to work on it
    Character* people_ = nullptr;
};

class RepetitiveJob : public Job {
public:
    RepetitiveJob(const std::string& name, Position tile_position, const std::string& icon_type, int build_time_ms, int repetition=9999);
    virtual ~RepetitiveJob() {}

    virtual bool isRepetitive();
    virtual void repeat() override;
    int repetition() const { return repetition_; }

private:
    int original_build_time_ms_;
    int repetition_;
};

/********************************************************************/

class BuildWallJob : public Job {
public:
    BuildWallJob(
        Position tile_position, int build_time_ms
    ) : Job(BUILDWALL, tile_position, "buttons/wall_tool", build_time_ms)
    {
    }
};

class BuildDoorJob : public Job {
public:
    BuildDoorJob(
        Position tile_position, int build_time_ms
    ) : Job(BUILDDOOR, tile_position, "buttons/door_tool", build_time_ms)
    {
    }
};

class DemolishWallJob : public Job {
public:
    DemolishWallJob(
        Position tile_position, int build_time_ms
    ) : Job(DEMOLISHWALL, tile_position, "buttons/demolish_tool", build_time_ms)
    {
    }
};

class BuildFloorJob : public Job {
public:
    BuildFloorJob(
        Position tile_position, int build_time_ms
    ) : Job(BUILDFLOOR, tile_position, "buttons/foundation_tool", build_time_ms)
    {
    }
};

class DemolishFloorJob : public Job {
public:
    DemolishFloorJob(
        Position tile_position, int build_time_ms
    ) : Job(DEMOLISHFLOOR, tile_position, "buttons/demolish_foundation_tool", build_time_ms)
    {
    }
};

class DemolishDoorJob : public Job {
public:
    DemolishDoorJob(
        Position tile_position, int build_time_ms
    ) : Job(DEMOLISHDOOR, tile_position, "buttons/demolish_tool", build_time_ms)
    {
    }
};

class ExtractJob : public RepetitiveJob {
public:
    ExtractJob(
        Position tile_position, const std::string& icon_type, int build_time_ms, int repetition
    ) : RepetitiveJob(EXTRACT, tile_position, icon_type, build_time_ms, repetition)
    {
    }
};

class CleanJob : public RepetitiveJob {
public:
    CleanJob(
        Position tile_position, int build_time_ms
    ) : RepetitiveJob(CLEAN, tile_position, "buttons/clean_tool", build_time_ms)
    {
    }
};

class BuildObjectJob : public Job {
public:
    BuildObjectJob(
        Position tile_position, const std::string& object_name, int build_time_ms
    ) : Job(BUILDOBJECT, tile_position, "objects/"+object_name, build_time_ms), object_name_(object_name)
    {
    }

    Object* getObject() const;
    const std::string& objectName() const {
        return object_name_;
    }
private:
    std::string object_name_;
};

class UnbuildObjectJob : public Job {
public:
    UnbuildObjectJob(
        Position tile_position, int build_time_ms
    ) : Job(DEMOLISHOBJECT, tile_position, "buttons/uninstall_tool", build_time_ms)
    {
    }
};

/********************************************************************/

class JobMgr {
public:
    JobMgr(SDL_Renderer* renderer);
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

    SDL_Texture* getTexture(const std::string& icon_type);
    void render(const Job& job, SDLCamera* camera, const SDL_Rect& rect);

private:
    SDL_Renderer* renderer_;
    std::list<Job*> jobs_;
    std::map<std::string, SDL_Texture*> map_of_jobs_;
};

/********************************************************************/

#endif // job_h
