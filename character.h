#ifndef character_h
#define character_h

#include "utility.h"
#include "items.h"

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <ctime>
#include <chrono>

class ActionBase;
class SDLCamera;
class Job;

/********************************************************************/

class Item {
public:
    Item(std::string name, Position position, int image_id);
    ~Item() {}

    void setTilePosition(Position position) {
        tile_position_ = position;
    }
    Position tilePosition() const {
        return tile_position_;
    }

    const std::string& name() const {
        return name_;
    }

    int imageIdForArchive() const { return image_id_; }

protected:
    std::string name_;
    const int image_id_; // for archive
    std::vector<SDL_Texture*> images_;
    Position tile_position_;
};

/********************************************************************/

class DynamicItem : public Item {
public:
    DynamicItem(std::string name, Position position, int image_id);
    ~DynamicItem() {}

    virtual void render(SDLCamera* camera, const SDL_Rect& rect) = 0;
    // render(surface, position)
    // if self.images.size() > 0:
    //     surface.blit(self.images[0], position)

    virtual void animate(double delta_ms);

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

    bool hasAction() const { return action_ != nullptr; }
    void setAction(ActionBase* action, std::string description);
    ActionBase* action() const { return action_; }
    void cancelAction();
    const std::string& actionDescription() const {
        return action_description_;
    }

protected:
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

    int activityPercent() const {
        return activity_percent_;
    }
    void setActivityPercent(int value) {
        activity_percent_ = value;
    }

    int maxCarry() const;
    int maxCarriable(const BasicItem& item = BasicItem::null()) const;
    void carryItem(const BasicItem& item, int nb=1);
    BasicItem dropItem();
    const std::vector<BasicItem>& carriedItems() const { return carried_items_; }
    void releaseItems();

protected:
    Direction direction_;
    int activity_percent_ = 0; // 0 to 100
    int max_carry_ = 5;
    std::vector<BasicItem> carried_items_;
};

/********************************************************************/

class GameBoard;

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

    void animate(GameBoard* board, double delay_ms);

    std::vector<Character*>& group();

    void add(Character* people);

    Character* getNextRobot();

protected:
    std::vector<Character*> group_;
    int next_robot_idx_ = -1;
};

/********************************************************************/

#endif // character_h
