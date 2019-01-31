#ifndef chest_h
#define chest_h

#include <vector>
#include <string>

#include "utility.h"
#include "translator.h"
#include "items.h"

class SDL_Texture;

/********************************************************************/

class SDLCamera;

class Object {
public:
    Object(const std::string& icon_name, const std::string& user_name, const std::string& name);
    ~Object() {}

    SDL_Texture* getTexture(SDLCamera* camera, int index = 0);
    virtual void render(SDLCamera* camera, const SDL_Rect& rect);

    int pixel_width() const  { return width_; }
    int pixel_height() const  { return height_; }

    const std::string& userName() const {
        return user_name_;
    }
    const std::string& name() const {
        return name_;
    }
    virtual const std::string tooltip() const {
        return tr(user_name_);
    }

protected:
    std::string icon_name_;
    std::vector<SDL_Texture*> images_;
    int width_;
    int height_;
    std::string user_name_;
    std::string name_;
};

class Chest : public Object {
public:
    Chest(int size = 16);
    virtual ~Chest() {}

    virtual void render(SDLCamera* camera, const SDL_Rect& rect) override;

    const CountedItem& item(int index) const {
        return items_[index];
    }

    const std::vector<CountedItem>& items() const { return items_; }

    int addItem(const BasicItem& item, int count = 1);
    int removeItem(const BasicItem& item, int count = 1);

    int sizeAvailable() const {
        return max_size_ - items_.size();
    }

    virtual const std::string tooltip() const override;

protected:
    int max_size_;
    std::vector<CountedItem> items_;
};

class WorkBench : public Object {
public:
    WorkBench();
    virtual ~WorkBench() {}
};

class StoneFurnace : public Object {
public:
    StoneFurnace();
    virtual ~StoneFurnace() {}
};

class ElectricFurnace : public Object {
public:
    ElectricFurnace();
    virtual ~ElectricFurnace() {}
};

/********************************************************************/

struct PositionObject {
    int x;
    int y;
    Object* object;
};

/********************************************************************/

#endif // chest_h
