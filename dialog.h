#ifndef dialog_h
#define dialog_h

#include "camera.h"
#include "object.h"
#include <SDL2/SDL.h>

class SDL_Surface;
class Character;
class SDLButton;
class SDLTextButton;
class Craft;

/********************************************************************/

class Dialog : public View {
public:
    Dialog(int x, int y, int width, int height);
    virtual ~Dialog();

    void setBackgroundColor(const SDL_Color& bgcolor);
    const SDL_Color& getBackgroundColor() const { return background_color_; }

    virtual void do_render(Camera* camera, double delay_in_ms) override;
    virtual bool handleEvent(Camera* camera) override;

    bool hasFocus(int mouse_x, int mouse_y);

    bool killMe() const { return kill_me_; }

    virtual Position tilePosition() const = 0;

    SDL_Rect getTitleRect(int extension=0) const;
    SDL_Rect getKillRect() const;
    SDL_Rect getMinimizeRect(int extension=0) const;
    SDL_Rect getCenterRect() const;

protected:
    bool buttonClicked(SDLButton* button, Position mouse_position);

protected:
    int x_;
    int y_;
    int width_;
    int height_;
    SDL_Color background_color_;
    SDL_Surface* surface_;
    bool kill_me_ = false;
    bool grabbing_ = false;
    int rel_grab_x_ = 0;
    int rel_grab_y_ = 0;
    bool minimized_ = false;
};

/********************************************************************/

class RobotDialog : public Dialog {
public:
    RobotDialog(Character* robot, int mouse_x = 50, int mouse_y = 50);
    virtual ~RobotDialog();

    Character* robot() const { return robot_; }

    virtual Position tilePosition() const override;

    virtual void do_render(Camera* camera, double delay_in_ms) override;
    virtual bool handleEvent(Camera* camera) override;

private:
    Character* robot_ = nullptr;
    SDLTextButton* drop_button_ = nullptr;
    std::vector<SDLButton*> buttons_;
};

/********************************************************************/

class ObjectDialog : public Dialog {
public:
    virtual ~ObjectDialog();

    Object* object() const { return object_; }

    virtual Position tilePosition() const override;

    virtual void do_render(Camera* camera, double delay_in_ms) override;

    static ObjectDialog* createDialog(Object* object, int x, int y);

protected:
    ObjectDialog(Object* object, int mouse_x = 50, int mouse_y = 50);

protected:
    Object* object_;
};

class CommandCenterDialog : public ObjectDialog {
public:
    CommandCenterDialog(Object* object, int mouse_x = 50, int mouse_y = 50);
    virtual ~CommandCenterDialog();

    virtual void do_render(Camera* camera, double delay_in_ms) override;
protected:
    std::vector<SDLButton*> buttons_;
    CommandCenter* command_center_;
};

struct CraftOccButton {
    Craft* craft;
    int occurrence;
    SDLButton* button;
};

class CrafterDialog : public ObjectDialog {
public:
    CrafterDialog(Object* object, int mouse_x = 50, int mouse_y = 50);
    virtual ~CrafterDialog();

    virtual void do_render(Camera* camera, double delay_in_ms) override;
    virtual bool handleEvent(Camera* camera) override;

protected:
    virtual void execute();
    void addCraft(int occ=1);
    void selectCraft(Craft* craft);

protected:
    std::string craft_button_label_;
    SDLTextButton* one_button_ = nullptr;
    SDLTextButton* ten_button_ = nullptr;
    SDLTextButton* craft_button_ = nullptr;
    Craft* selected_craft_ = nullptr;
    std::vector< std::pair<Craft*,SDLButton*> > recipe_buttons_;
    std::vector< CraftOccButton > craft_buttons_;
};

class SmelterDialog : public CrafterDialog {
public:
    SmelterDialog(Object* object, int mouse_x = 50, int mouse_y = 50);
    virtual ~SmelterDialog();

protected:
    virtual void execute() override;

};

/********************************************************************/

#endif // dialog_h
