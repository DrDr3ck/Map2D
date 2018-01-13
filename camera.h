#ifndef camera_h
#define camera_h

#include "map.h"
#include <list>

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

class Button {
public:
    Button(int x,int y, int w, int h);
    virtual ~Button() {}

    bool isActive() { return active_; }
    virtual void activate() { active_ = true; }
    virtual void deactivate() { active_ = false; }

    bool isVisible() const { return visible_; }
    void visible(bool value) { visible_ = value; }

    bool mouseOverButton(int mouse_x, int mouse_y) const;

    void hasFocus(bool value) { focus_ = value; }
    bool hasFocus() const { return focus_; }

private:
    bool active_;
    bool visible_;
    bool focus_;
    int x_; // position x
    int y_; // position y
    int w_; // width
    int h_; // height
};

/********************************************************************/

class ButtonManager : public View {
public:
    ButtonManager() {}
    virtual ~ButtonManager() {}

    void addButton(Button* button) { buttons_.push_back(button); }
    void removeButton(Button* button) { buttons_.remove(button); }

protected:
    std::list<Button*> buttons_;
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

    virtual void do_quit() const = 0;
    bool quit() const { return quit_; }
    void set_quit() { quit_ = true; }

    int mouse_x() const { return mouse_x_; }
    int mouse_y() const { return mouse_y_; }

protected:
    std::list<View*> views_;
    bool pause_;
    bool quit_;
    int mouse_x_;
    int mouse_y_;
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
