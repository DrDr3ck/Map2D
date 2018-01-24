#include "button.h"

#include <iostream>

/***********************************/

Button::Button(std::string button_name, int x,int y, int w, int h) : button_name_(button_name), x_(x), y_(y), w_(w), h_(h) {
    visible_ = true;
    active_ = false;
    focus_ = false;
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

/*!
 * Shows all buttons of the menu
 */
void MenuButton::show() {
    visible_ = true;
    for( auto b : buttons_ ) {
        b->visible(visible_);
    }
}

/*!
 * Hides all buttons of the menu
 */
void MenuButton::hide() {
    visible_ = false;
    for( auto b : buttons_ ) {
        b->visible(visible_);
    }
}

void MenuButton::addButton(Button* button) {
    buttons_.push_back(button);
    button->visible( isVisible() );
    resetButtons();
}

void MenuButton::resetButtons() {
    int margin = 5;
    int max_width_pxl = margin;
    int max_height_pxl = margin;
    int row = 0;
    int col = 0;
    int cur_width_pxl = margin;
    int cur_height_pxl = margin;
    for( Button* button : buttons_ ) {
        if( col == max_column_ ) {
            row++;
            col = 0;
            if( cur_width_pxl > max_width_pxl ) {
                max_width_pxl = cur_width_pxl;
            }
            cur_width_pxl = margin;

            max_height_pxl += cur_height_pxl;
            cur_height_pxl = margin;
        }
        col++;
        if( cur_height_pxl < margin + button->h() ) {
            cur_height_pxl = margin + button->h();
        }
        button->setPosition(x_+cur_width_pxl, y_+max_height_pxl);
        cur_width_pxl += button->w();
        cur_width_pxl += margin;
        if( cur_height_pxl < margin + button->h() ) {
            cur_height_pxl = margin + button->h();
        }
    }

    if( cur_width_pxl > max_width_pxl ) {
        max_width_pxl = cur_width_pxl;
    }
    max_height_pxl += cur_height_pxl;

    width_ = max_width_pxl;
    height_ = max_height_pxl;
}

/**************************************/

