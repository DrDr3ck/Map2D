#ifndef button_h
#define button_h

#include "camera.h"
#include <list>

class Camera;

/********************************************************************/

class Button {
public:
    Button(int x,int y, int w=32, int h=32);
    virtual ~Button() {}

    bool isActive() { return active_; }
    virtual void activate() { active_ = true; }
    virtual void deactivate() { active_ = false; }

    bool isVisible() const { return visible_; }
    void visible(bool value) { visible_ = value; }

    bool mouseOverButton(int mouse_x, int mouse_y) const;

    void hasFocus(bool value) { focus_ = value; }
    bool hasFocus() const { return focus_; }

    void setSize(int w, int h) { w_ = w; h_ = h; }
    void setPosition(int x,int y) { x_ = x; y_ = y; }

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

#endif // camera_h
