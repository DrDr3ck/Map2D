#include "camera.h"

#include <iostream>

/***********************************/

Camera::Camera() {
    pause_ = true;
    quit_ = false;
}

void Camera::addView(View* view) {
    views_.push_back(view);
}

void Camera::removeView(View* view) {
    views_.remove(view);
}

void Camera::render() {
    for( auto view : views_ ) {
        view->do_render(this);
    }
}

void Camera::handleEvent() {
    for( auto view : views_ ) {
        view->handleEvent(this);
    }
}

/**************************************/

