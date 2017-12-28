#ifndef tests_h
#define tests_h

#include "map.h"
#include <list>
#include <string>

class Test {
public:
    Test(std::string name) : test_name_(name) {}
    ~Test() {}

    virtual bool do_execute() = 0;

    const std::string& name() const { return test_name_; }
private:
    std::string test_name_;
};

class CheckingTest : public Test {
public:
    CheckingTest() : Test("CheckingTest") {}

    virtual bool do_execute() override;
};

class TileTest : public Test {
public:
    TileTest() : Test("TileTest") {}

    virtual bool do_execute() override;
};

class MapTest : public Test {
public:
    MapTest() : Test("MapTest") {}

    virtual bool do_execute() override;
};

class TestManager {
public:
    TestManager();
    ~TestManager() {}

    bool execute();
    void addTest(Test* test);

private:
    std::list<Test*> tests_;
};

#endif // tests_h
