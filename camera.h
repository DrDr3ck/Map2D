#ifndef camera_h
#define camera_h

#include "map.h"
#include <list>

class Camera;

class View {
public:
    View() {}
    virtual ~View() {}

    virtual void do_render(Camera* camera) = 0;
    virtual void handleEvent(Camera* camera) = 0;
};

/********************************/

class Camera {
public:
    Camera();

    virtual bool valid() const = 0;

    void addView(View* view);
    void removeView(View* view);

    virtual void render();
    virtual void handleEvent();

    virtual void do_quit() const = 0;
    bool quit() const { return quit_; }

protected:
    std::list<View*> views_;
    bool pause_;
    bool quit_;
};

#endif // camera_h
