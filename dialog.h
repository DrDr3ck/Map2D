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

protected:
    int x_;
    int y_;
    int width_;
    int height_;
    SDL_Surface* surface_;
};

/********************************************************************/

#endif // dialog_h
