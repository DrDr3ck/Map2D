#ifndef camera_h
#define camera_h

#include "utility.h"
#include <list>
#include <vector>

class Camera;
class RobotDialog;
class ObjectDialog;
class Character;
class Object;

/********************************************************************/

class View {
public:
    View() {}
    virtual ~View() {}

    virtual void do_render(Camera* camera, double delay_in_ms) = 0;
    virtual bool handleEvent(Camera* camera) = 0;
};

/********************************************************************/

class Camera {
public:
    Camera(int width, int height);
    virtual ~Camera() {}

    virtual bool valid() const = 0;

    void addView(View* view);
    void removeView(View* view);

    RobotDialog* findRobotDialog(Character* robot) const;
    ObjectDialog* findObjectDialog(Object* object) const;

    virtual void render(double delay_in_ms);
    virtual bool handleEvent();

    float scale() const { return scales_[scale_idx_]; }
    float speed() const { return speeds_[scale_idx_]; }
    void zoom(bool up);

    int width() const { return width_; }
    int height() const { return height_; }
    void setSize(int width, int height);

    virtual void onMouseMove(int x, int y);
    virtual void onMouseWheelScroll(int x, int y);

    bool quit() const { return quit_; }
    void set_quit() { quit_ = true; }

    bool isInPause() const { return pause_; }

    int mouse_x() const { return mouse_x_; }
    int mouse_y() const { return mouse_y_; }

    int wheel_x() const { return wheel_x_; }
    int wheel_y() const { return wheel_y_; }

    static Camera* cur_camera;

protected:
    std::list<View*> views_;
    bool pause_;
    bool quit_;
    bool lctrl_down_;
    bool rctrl_down_;
    int mouse_x_;
    int mouse_y_;
    int wheel_x_;
    int wheel_y_;
    int scale_idx_;
    std::vector<float> scales_;
    std::vector<float> speeds_;

    int width_;
    int height_;
};

/********************************************************************/

class Tool {
public:
    Tool() {}
    virtual ~Tool() {}

    virtual void handleEvent() = 0;
    virtual void mousePressed(int button) = 0;
    virtual void keyPressed(const std::string& key) = 0;
    virtual void mouseMoved(int mouse_x, int mouse_y) = 0;
    virtual void mouseReleased(int button) = 0;
};

/********************************************************************/

#endif // camera_h
