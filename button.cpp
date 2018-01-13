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

