#ifndef items_h
#define items_h

#include <string>

/********************************************************************/

class BasicItem {
public:
    BasicItem(std::string name) : name_(name) {}
    BasicItem(const BasicItem& item) : name_(item.name()) {}
    ~BasicItem() {}

    bool operator==(const BasicItem& rhs) const {
        return (name_ == rhs.name());
    }

    const std::string& name() const {
        return name_;
    }

protected:
    std::string name_;
};

/********************************************************************/

class CountedItem {
public:
    CountedItem(const BasicItem& item, int count = 0);
    ~CountedItem();

    int addItem(int count = 1);
    int removeItem(int count = 1);

    const BasicItem& item() const { return item_; }
    int count() const { return count_; }
protected:
    BasicItem item_;
    int count_;
};

/********************************************************************/

#endif // items_h
