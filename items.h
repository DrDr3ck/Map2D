#ifndef items_h
#define items_h

#include <string>

class SDL_Texture;

/********************************************************************/

class BasicItem {
public:
    BasicItem(std::string name) : name_(name), texture_(nullptr) {}
    BasicItem(const BasicItem& item) : name_(item.name()), texture_(nullptr) {}
    ~BasicItem() {}

    bool operator==(const BasicItem& rhs) const {
        return (name_ == rhs.name());
    }

    const std::string& name() const {
        return name_;
    }

    static BasicItem& null();

    SDL_Texture* texture();
protected:
    std::string name_;
    SDL_Texture* texture_;
};

/********************************************************************/

class CountedItem {
public:
    CountedItem();
    CountedItem(const BasicItem& item, int count = 0);
    ~CountedItem();

    int addItem(int count = 1);
    int removeItem(int count = 1);

    const BasicItem& item() const { return item_; }
    int count() const { return count_; }

    bool isNull() const;

    SDL_Texture* texture() const {
        CountedItem* non_const = const_cast<CountedItem*>(this);
        return non_const->item_.texture();
    }
protected:
    BasicItem item_;
    int count_;
};

/********************************************************************/

#endif // items_h
