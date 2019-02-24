#ifndef action_h
#define action_h

#include "utility.h"

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <ctime>
#include <chrono>

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
    bool is_valid_ = true;
};

/********************************************************************/

class Character;

//_no_action_descs = [
//    tr("Looking around, just for fun..."),
//    tr("Need a job"),
//    tr("Gets bored"),
//    tr("Dreaming")
//]

class NoAction : public ActionBase {
public:
    NoAction(Character* character);

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

class GameBoard;
class Job;

class BuildAction : public ActionBase {
public:
    BuildAction(GameBoard* game_board, Character* people, Job* job);
    virtual ~BuildAction();

    virtual void preAction() override;
    virtual bool spentTime(double time_spent) override;
    virtual void postAction() override;

    void reset();

private:
    GameBoard* game_board_;
    Character* people_;
    Job* job_;
    ActionBase* action_;
    std::chrono::steady_clock::time_point start_time_;
};

class ExtractAction : public ActionBase {
public:
    ExtractAction(GameBoard* game_board, Character* people, Job* job);
    virtual ~ExtractAction();

    virtual void preAction() override;
    virtual bool spentTime(double time_spent) override;
    virtual void postAction() override;

private:
    GameBoard* game_board_;
    Character* people_;
    Job* job_;
    ActionBase* action_;
    bool isValid_;
    std::chrono::steady_clock::time_point start_time_;
};

class CleanAction : public ActionBase {
public:
    CleanAction(GameBoard* game_board, Character* people, Job* job);
    virtual ~CleanAction();

    virtual void preAction() override;
    virtual bool spentTime(double time_spent) override;
    virtual void postAction() override;

private:
    GameBoard* game_board_;
    Character* people_;
    Job* job_;
    ActionBase* action_;
    bool isValid_;
    std::chrono::steady_clock::time_point start_time_;
};

/********************************************************************/

#endif // action_h
