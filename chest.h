#ifndef chest_h
#define chest_h

#include <vector>
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

class Chest {
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

#endif // chest_h
