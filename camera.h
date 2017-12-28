#ifndef camera_h
#define camera_h

#include "map.h"
#include <list>

class View {
public:
    View() {}
    virtual ~View() {}

    virtual void do_render() = 0;
};

class MapView : public View {
public:
    MapView(MapData* data);
    ~MapView() { data_ = nullptr; }

    virtual void do_render() override;
private:
    MapData* data_;
};

class Camera {
public:
    Camera() {}
    ~Camera() {}

    void addView(View* view);
    void removeView(View* view);

    void render();

private:
    std::list<View*> views_;
};

#endif // camera_h
