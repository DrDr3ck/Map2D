#include "button.h"

#include <iostream>

/***********************************/

Button::Button(int x,int y, int w, int h) : x_(x), y_(y), w_(w), h_(h) {
    visible_ = true;
}

/*!
 * \return true if mouse position is over the Button
 */
bool Button::mouseOverButton(int mouse_x, int mouse_y) const {
    if( mouse_x < x_ ) return false;
    if( mouse_x > x_+w_ ) return false;
    if( mouse_y < y_ ) return false;
    if( mouse_y > y_+h_ ) return false;
    return true;
}

/**************************************/

MenuButton::MenuButton(int max_column, int x, int y)
 : max_column_(max_column), x_(x), y_(y), visible_(false), width_(10), height_(10)
{
}

void MenuButton::show() {
    visible_ = true;
    for( auto b : buttons_ ) {
        b->visible(visible_);
    }
}

void MenuButton::hide() {
    visible_ = false;
    for( auto b : buttons_ ) {
        b->visible(visible_);
    }
}

void MenuButton::addButton(Button* button) {
    buttons_.push_back(button);
    button->visible( isVisible() );
    width_ += button->w();
    height_ += button->h();
    button->setPosition(x_+5, y_+5);
}

/**************************************/

