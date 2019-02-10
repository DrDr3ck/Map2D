#ifndef chest_h
#define chest_h

#include <vector>
#include <string>

#include "utility.h"
#include "translator.h"
#include "items.h"

class SDL_Texture;
class Craft;

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

    bool inPause() const { return in_pause_; }
    void togglePause(bool value) { in_pause_ = value; }

    bool hasIngredients() const;
    void checkIngredients();

    virtual void animate(double delta_ms);

    bool hasCrafts() const { return is_crafter_; }
    void addCraft(Craft* craft, int occ=1);

    virtual int getNodeCount() const;
    virtual const std::string getNodeName(int node_index) const;
    virtual int getAttributeCount(int node_index) const;
    virtual const std::string getAttributeName(int node_index,int attr_index) const;
    virtual const std::string getAttributeValue(int node_index,int attr_index) const;
    virtual const std::string getNodeValue(int node_index) const;
    virtual const std::string getNodeString(int node_index) const;

    virtual void setNode(const std::string& node_name, std::vector<std::pair<std::string, std::string>> attributes, const std::string& value);

protected:
    std::string icon_name_;
    std::vector<SDL_Texture*> images_;
    int width_;
    int height_;
    std::string user_name_;
    std::string name_;
    bool is_crafter_ = true;
    std::vector< std::pair<Craft*, int> > crafts_;
    Craft* cur_craft_ = nullptr;
    bool has_ingredients_ = false;
    int craft_time_ms_ = 0;
    bool in_pause_ = false;
};

#define Attr std::pair<std::string,std::string>

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

    virtual int getNodeCount() const override;
    virtual const std::string getNodeName(int node_index) const override;
    virtual int getAttributeCount(int node_index) const override;
    virtual const std::string getAttributeName(int node_index,int attr_index) const override;
    virtual const std::string getAttributeValue(int node_index,int attr_index) const override;
    virtual const std::string getNodeValue(int node_index) const override;
    virtual const std::string getNodeString(int node_index) const;

    virtual void setNode(const std::string& node_name, std::vector<Attr> attributes, const std::string& value) override;

protected:
    int max_size_;
    std::vector<CountedItem> items_;
};

class WorkBench : public Object {
public:
    WorkBench();
    virtual ~WorkBench() {}
};

class Furnace : public Object {
public:
    Furnace(const std::string& icon_name, const std::string& user_name, const std::string& name);
    virtual ~Furnace() {}

    virtual void animate(double delta_ms) override;

    bool needFuel();
    void getFuel();
private:
    CountedItem fuel_;
    int fuel_time_ms_ = 0;
};

class StoneFurnace : public Furnace {
public:
    StoneFurnace();
    virtual ~StoneFurnace() {}
};

class ElectricFurnace : public Furnace {
public:
    ElectricFurnace();
    virtual ~ElectricFurnace() {}
};

class Breaker : public Object {
public:
    Breaker();
    virtual ~Breaker() {}
};

/********************************************************************/

struct PositionObject {
    int x;
    int y;
    Object* object;
};

/********************************************************************/

#endif // chest_h
