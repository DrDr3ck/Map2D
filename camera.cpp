#include "camera.h"
#include "session.h"

#include <iostream>

/***********************************/

Camera::Camera(int width, int height) : width_(width), height_(height) {
    pause_ = Session::instance()->getBoolean("*pause_when_launched");
    quit_ = false;
    scales_ = { 0.5, 0.6, 0.8, 1., 1.2, 1.5, 1.9, 2.4, 2.8 };
    scale_idx_ = 3; // so that scales_[scale_idx_] = 1
}

void Camera::addView(View* view) {
    views_.push_back(view);
}

void Camera::removeView(View* view) {
    views_.remove(view);
}

/*!
 * Increases or decreases the scale factor.
 */
void Camera::zoom(bool up) {
    if( up ) {
        scale_idx_++;
        if( scale_idx_ >= int(scales_.size()) ) {
            scale_idx_ = scales_.size() - 1;
        }
    } else {
        scale_idx_--;
        if( scale_idx_ < 0 ) {
            scale_idx_ = 0;
        }
    }
}

void Camera::onMouseMove(int x, int y) {
    mouse_x_ = x;
    mouse_y_ = y;
}

void Camera::onMouseWheelScroll(int x, int y) {
    wheel_x_ = x;
    wheel_y_ = y;
    if( y == 1 ) {
        zoom(true);
    }
    if( y == -1 ) {
        zoom(false);
    }
}

void Camera::render(double delay_in_ms) {
    for( auto view : views_ ) {
        view->do_render(this, delay_in_ms);
    }
}

void Camera::handleEvent() {
    for( auto view : views_ ) {
        view->handleEvent(this);
    }
}

/**************************************/

