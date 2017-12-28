#include "camera.h"

MapView::MapView(MapData* data) : data_(data) {
}

void MapView::do_render() {
    // TODO
}

/***********************************/

void Camera::addView(View* view) {
    views_.push_back(view);
}

void Camera::removeView(View* view) {
    views_.remove(view);
}

void Camera::render() {
    for( auto view : views_ ) {
        view->do_render();
    }
}
