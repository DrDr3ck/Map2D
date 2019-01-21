#ifndef chest_h
#define chest_h

#include <vector>
#include <string>

#include "utility.h"
#include "items.h"

class SDL_Texture;

/********************************************************************/

class SDLCamera;

class Object {
public:
    Object(const std::string& icon_name, const std::string& object_name);
    ~Object() {}

    SDL_Texture* getTexture(SDLCamera* camera, int index = 0);
    void render(SDLCamera* camera, const SDL_Rect& rect);

    int pixel_width() const  { return width_; }
    int pixel_height() const  { return height_; }

    const std::string& name() const {
        return name_;
    }

protected:
    std::string icon_name_;
    std::vector<SDL_Texture*> images_;
    int width_;
    int height_;
    std::string name_;
};

class Chest : public Object {
public:
    Chest(int size = 16);
    virtual ~Chest() {}

    const CountedItem& item(int index) const {
        return items_[index];
    }

    int addItem(const BasicItem& item, int count = 1);
    int removeItem(const BasicItem& item, int count = 1);

    int sizeAvailable() const {
        return max_size_ - items_.size();
    }

protected:
    int max_size_;
    std::vector<CountedItem> items_;
};

/********************************************************************/

struct PositionObject {
    int x;
    int y;
    Object* object;
};

/********************************************************************/

#endif // chest_h
