#ifndef dialog_h
#define dialog_h

#include "camera.h"

class SDL_Surface;

/********************************************************************/

class Dialog : public View {
public:
    Dialog(int x, int y, int width, int height);
    virtual ~Dialog();

    void setBackgroundColor(int R, int G, int B, int A=255);

    virtual void do_render(Camera* camera, double delay_in_ms);
    virtual void handleEvent(Camera* camera);

    bool hasFocus(int mouse_x, int mouse_y);

    bool killMe() const { return kill_me_; }

protected:
    int x_;
    int y_;
    int width_;
    int height_;
    int red_ = 210;
    int green_ = 210;
    int blue_ = 210;
    int alpha_ = 255;
    SDL_Surface* surface_;
    bool kill_me_ = false;
    bool grabbing_ = false;
    int rel_grab_x_ = 0;
    int rel_grab_y_ = 0;
};

/********************************************************************/

#endif // dialog_h
