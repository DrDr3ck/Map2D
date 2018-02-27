#ifndef character_h
#define character_h

#include "map.h"

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <ctime>
#include <chrono>

class ActionBase;
class SDLCamera;

/********************************************************************/

class DynamicItem {
public:
    DynamicItem(std::string name, Position position, int image_id);
    ~DynamicItem() {}

    virtual void render(SDLCamera* camera, const SDL_Rect& rect) = 0;
    // render(surface, position)
    // if self.images.size() > 0:
    //     surface.blit(self.images[0], position)

    virtual void animate(double delta_ms);

    void setTilePosition(Position position) {
        tile_position_ = position;
    }
    Position tilePosition() const {
        return tile_position_;
    }

    void setTimeSpent(double value) {
        time_spent_ = value;
    }

    bool validPixelPosition() const {
        return pixel_position_.x != -1;
    }
    Position pixelPosition() const {
        return pixel_position_;
    }
    void setPixelPosition(int x, int y) {
        pixel_position_ = {x,y};
    }

    const std::string& name() const {
        return name_;
    }

    void setAction(ActionBase* action, std::string description);
    const std::string& actionDescription() const {
        return action_description_;
    }

protected:
    std::string name_;
    std::vector<SDL_Texture*> images_;
    Position tile_position_;
    Position pixel_position_;
    double time_spent_ = 0;

    ActionBase* action_ = nullptr;
    std::string action_description_;
};

/********************************************************************/

class Character : public DynamicItem {
public:
    Character(std::string name, Position tile_position, int image_id);
    virtual ~Character() {}

    virtual void render(SDLCamera* camera, const SDL_Rect& rect) override;

    Direction direction() const {
        return direction_;
    }
    void setDirection(int x, int y);

protected:
    Direction direction_;
    int activity_percent_ = 0; // 0 to 100
};

/********************************************************************/

class ActionBase {
public:
    ActionBase() : next_action_(nullptr) {}
    virtual ~ActionBase() {}

    virtual void preAction() {}
    virtual bool spentTime(double time_spent) = 0;
    virtual void postAction() {}

protected:
    ActionBase* next_action_ = nullptr;
};

/********************************************************************/
//_no_action_descs = [
//    tr("Looking around, just for fun..."),
//    tr("Need a job"),
//    tr("Gets bored"),
//    tr("Dreaming")
//]

class NoAction : public ActionBase {
public:
    NoAction(Character* character) : ActionBase(), character_(character) {
        max_time_spent_ = std::rand() % 5; // in seconds
        if( max_time_spent_ > 2 )
            character->setAction(this, std::string("Need a job")); // TODO: random choice from list of no action descs
        else
            character->setAction(this, std::string("Dreaming")); // TODO: random choice from list of no action descs
    }

    virtual bool spentTime(double time_spent) override;

protected:
    Character* character_;
    Position origin_;
    double max_time_spent_;
};

/********************************************************************/

class MoveAction : public ActionBase {
public:
    MoveAction(Character* character, std::vector<Position>& path_in_tile, int tile_size);
    ~MoveAction() {}

    virtual bool spentTime(double time_spent) override;
    virtual void postAction() override;

protected:
    void create_animation_path(std::vector<Position>& path_in_tile, int tile_size);
    Position get_position_in_pixel();

private:
    Character* character_;
    Position origin_;
    std::vector<Position> path_in_pixel_;
    std::chrono::steady_clock::time_point start_time_;
    float speed_;
    float activity_percent_;
    Position destination_;
    bool is_finished_;

    Position prev_position_;
    Position next_position_;
};

/********************************************************************/

class CharacterSetLib {
private:
  CharacterSetLib();
  ~CharacterSetLib();

public:
    static CharacterSetLib* instance();
    static void kill();

    static SDL_Texture* getTextureFromCharacter(int character_id);

    SDL_Surface* characters() { return characters_surface_; }
    std::map<int, SDL_Texture*>& mapOfCharacters() { return mapOfCharacters_; }
    void init(SDL_Renderer* renderer);
private:
    static CharacterSetLib* singleton_;
    SDL_Surface* characters_surface_;
    std::map<int, SDL_Texture*> mapOfCharacters_;
};

class PeopleGroup {
public:
    PeopleGroup();
    ~PeopleGroup();

    void animate(double delay_ms);

    std::vector<Character*>& group();

protected:
    std::vector<Character*> group_;
};

/********************************************************************/

#endif // character_h
