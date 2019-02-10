#ifndef dialog_h
#define dialog_h

#include "camera.h"
#include "object.h"
#include <SDL2/SDL.h>

class SDL_Surface;
class Character;
class SDLButton;
class SDLTextButton;

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
private:
    Character* robot_;
};

/********************************************************************/

class ObjectDialog : public Dialog {
public:
    virtual ~ObjectDialog();

    Object* object() const { return pobject_.object; }

    virtual Position tilePosition() const override;

    virtual void do_render(Camera* camera, double delay_in_ms) override;
    virtual bool handleEvent(Camera* camera) override;

    static ObjectDialog* createDialog(PositionObject object, int x, int y);

protected:
    ObjectDialog(PositionObject pobject, int mouse_x = 50, int mouse_y = 50);
    virtual void execute();

    bool buttonClicked(SDLButton* button, Position mouse_position);

private:
    PositionObject pobject_;
    SDLTextButton* one_button_;
    SDLTextButton* ten_button_;
    SDLTextButton* craft_button_;
};

/********************************************************************/

#endif // dialog_h
