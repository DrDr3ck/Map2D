#ifndef character_h
#define character_h

#include <SDL2/SDL.h>
#include <map>
#include "map.h"

class ActionBase;

/********************************************************************/

class DynamicItem {
public:
    DynamicItem(std::string name, Position position, int image_id);
    ~DynamicItem() {}

    virtual void render() {} // TODO
    // render(surface, position)
    // if self.images.size() > 0:
    //     surface.blit(self.images[0], position)

    virtual void animate(double delta);

    Position tilePosition() const {
        return tile_position_;
    }

    void setAction(ActionBase* action, std::string description);
    const std::string& actionDescription() const {
        return action_description_;
    }

protected:
    std::string name_;
    Position tile_position_;
    Position pixel_position_;
    double time_spent_ = 0;

    ActionBase* action_ = nullptr;
    std::string action_description_;
};

class Character : public DynamicItem {
public:
    Character(std::string name, Position tile_position, int image_id);
    virtual ~Character() {}

    void setDirection(int x, int y);

protected:
    Direction direction_;
};

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
    double max_time_spent_;
};

/********************************************************************/

class CharacterSetLib {
private:
  CharacterSetLib();
  ~CharacterSetLib();

public:
    static CharacterSetLib* instance();
    static void kill();

    static SDL_Texture* getTextureFromCharacter(const Character& character, SDL_Renderer* renderer);

    SDL_Surface* characters() { return characters_surface_; }
    std::map<int, SDL_Texture*>& mapOfCharacters() { return mapOfCharacters_; }
private:
    static CharacterSetLib* singleton_;
    SDL_Surface* characters_surface_;
    std::map<int, SDL_Texture*> mapOfCharacters_;
};

/********************************************************************/

#endif // character_h
