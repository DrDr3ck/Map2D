#ifndef button_h
#define button_h

#include "camera.h"
#include <list>

class Camera;

/********************************************************************/

class Button {
public:
    Button(std::string button_name, int x,int y, int w=32, int h=32);
    virtual ~Button() {}

    bool isActive() { return active_; }
    virtual void activate() { active_ = true; }
    virtual void deactivate() { active_ = false; }

    bool isVisible() const { return visible_; }
    void visible(bool value) { visible_ = value; }

    bool mouseOverButton(int mouse_x, int mouse_y) const;

    void hasFocus(bool value) { focus_ = value; }
    bool hasFocus() const { return focus_; }

    virtual void setSize(int w, int h) { w_ = w; h_ = h; }
    virtual void setPosition(int x,int y) { x_ = x; y_ = y; }

    int x() const { return x_; }
    int y() const { return y_; }
    int w() const { return w_; }
    int h() const { return h_; }

    const std::string& buttonName() const { return button_name_; }
    void setButtonName(const std::string& name) { button_name_ = name; }

private:
    bool active_;
    bool visible_;
    bool focus_;
    std::string button_name_;
    int x_; // position x
    int y_; // position y
    int w_; // width
    int h_; // height
};

/********************************************************************/

class MenuButton {
public:
    MenuButton(int max_column, int x, int y);
    virtual ~MenuButton() {}

    bool isVisible() const { return visible_; }
    void show();
    void hide();

    void addButton(Button* button);

    int x() { return x_; }
    int y() { return y_; }
    int width() { return width_; }
    int height() { return height_; }

protected:
    void resetButtons();

private:
    int max_column_;
    int x_;
    int y_;
    bool visible_;
    std::list<Button*> buttons_;
    int width_;
    int height_;
};

/********************************************************************/

class ButtonManager : public View {
public:
    ButtonManager() {}
    virtual ~ButtonManager() {}

    void addButton(Button* button) { buttons_.push_back(button); }
    void removeButton(Button* button) { buttons_.remove(button); }

    void addMenuButton(MenuButton* menu) { menus_.push_back(menu); }
    void removeMenuButton(MenuButton* menu) { menus_.remove(menu); }

protected:
    std::list<Button*> buttons_;
    std::list<MenuButton*> menus_;
};

/********************************************************************/

#endif // button_h
