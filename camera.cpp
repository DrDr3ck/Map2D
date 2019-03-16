#include "camera.h"
#include "session.h"
#include "dialog.h"

#include <iostream>
#include <algorithm>

/***********************************/

Camera* Camera::cur_camera = nullptr;

Camera::Camera(int width, int height) : width_(width), height_(height) {
    pause_ = Session::instance()->getBoolean("*pause_when_launched", true);
    quit_ = false;
    lctrl_down_ = false;
    rctrl_down_ = false;
    scales_ = { 0.4, 0.5, 0.6, 0.8, 1.0, 1.2, 1.5, 1.9, 2.4, 2.8 };
    scale_idx_ = 3; // by default, use the index 3
    // find index of element 1.0
    std::vector<float>::iterator it = std::find(scales_.begin(), scales_.end(), 1.0);
    if( it != scales_.end() ) {
        scale_idx_ = std::distance(scales_.begin(), it);
    }
    speeds_ = { 5.0, 4.0, 3.5, 3.3, 2.5, 2.8, 3.0, 3.3, 3.8, 4.0 };
    cur_camera = this;
}

RobotDialog* Camera::findRobotDialog(Character* robot) const {
    for( auto view : views_ ) {
        RobotDialog* dialog = dynamic_cast<RobotDialog*>(view);
        if( dialog != nullptr && dialog->robot() == robot ) {
            return dialog;
        }
    }
    return nullptr;
}

ObjectDialog* Camera::findObjectDialog(Object* object) const {
    for( auto view : views_ ) {
        ObjectDialog* dialog = dynamic_cast<ObjectDialog*>(view);
        if( dialog != nullptr && dialog->object() == object ) {
            return dialog;
        }
    }
    return nullptr;
}

void Camera::addView(View* view) {
    if( view == nullptr ) return;
    views_.push_back(view);
}

void Camera::removeView(View* view) {
    if( view == nullptr ) return;
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

void Camera::setSize(int width, int height) {
    width_ = width;
    height_ = height;
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

bool Camera::handleEvent() {
    for (std::list<View*>::reverse_iterator rit=views_.rbegin(); rit!=views_.rend(); ++rit) {
        View* view = *rit;
        if( view->handleEvent(this) ) {
            return true;
        }
    }
    return false;
}

/**************************************/

