#ifndef camera_h
#define camera_h

#include "map.h"
#include <list>
#include <vector>

class Camera;

/********************************************************************/

class View {
public:
    View() {}
    virtual ~View() {}

    virtual void do_render(Camera* camera) = 0;
    virtual void handleEvent(Camera* camera) = 0;
};

/********************************************************************/

class Camera {
public:
    Camera();
    virtual ~Camera() {}

    virtual bool valid() const = 0;

    void addView(View* view);
    void removeView(View* view);

    virtual void render();
    virtual void handleEvent();

    float scale() const { return scales_[scale_idx_]; }
    void zoom(bool up);

    virtual void onMouseMove(int x, int y);
    virtual void onMouseWheelScroll(int x, int y);

    virtual void do_quit() const = 0;
    bool quit() const { return quit_; }
    void set_quit() { quit_ = true; }

    int mouse_x() const { return mouse_x_; }
    int mouse_y() const { return mouse_y_; }

    int wheel_x() const { return wheel_x_; }
    int wheel_y() const { return wheel_y_; }

protected:
    std::list<View*> views_;
    bool pause_;
    bool quit_;
    int mouse_x_;
    int mouse_y_;
    int wheel_x_;
    int wheel_y_;
    int scale_idx_;
    std::vector<float> scales_;
};

/********************************************************************/

class Tool {
public:
    Tool() {}
    virtual ~Tool() {}

    virtual void handleEvent() = 0;
    virtual void mousePress() = 0;
    virtual void mouseMotion() = 0;
    virtual void mouseRelease() = 0;
};

/********************************************************************/

#endif // camera_h
