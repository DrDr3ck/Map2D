#ifndef tests_h
#define tests_h

#include "map.h"
#include "character.h"
#include "object.h"
#include <list>
#include <string>

/*******************************/

class Test {
public:
    Test(std::string name);
    virtual ~Test() {}

    virtual bool do_execute() = 0;

    const std::string& name() const { return test_name_; }
private:
    std::string test_name_;
};

class CheckingTest : public Test {
public:
    CheckingTest();

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

class FontTest : public Test {
public:
    FontTest() : Test("FontTest") {}

    virtual bool do_execute() override;
};

class CharacterTest : public Test {
public:
    CharacterTest() : Test("CharacterTest") {}

    virtual bool do_execute() override;
};

class ActionTest : public Test {
public:
    ActionTest() : Test("ActionTest") {}

    virtual bool do_execute() override;
};

class JobTest : public Test {
public:
    JobTest() : Test("JobTest") {}

    virtual bool do_execute() override;
};

class ChestTest : public Test {
public:
    ChestTest() : Test("ChestTest") {}

    virtual bool do_execute() override;
};

class SessionTest : public Test {
public:
    SessionTest() : Test("SessionTest") {}

    virtual bool do_execute() override;
};

class PerlinTest : public Test {
public:
    PerlinTest() : Test("PerlinTest") {}

    virtual bool do_execute() override;
};

class XMLTest : public Test {
public:
    XMLTest() : Test("XMLTest") {}

    virtual bool do_execute() override;
};

class BiomeTest : public Test {
public:
    BiomeTest() : Test("BiomeTest") {}

    virtual bool do_execute() override;
};

class TranslatorTest : public Test {
public:
    TranslatorTest() : Test("TranslatorTest") {}

    virtual bool do_execute() override;
};

class CraftMgrTest : public Test {
public:
    CraftMgrTest() : Test("CraftMgrTest") {}

    virtual bool do_execute() override;
};

/*******************************/

class TestManager {
private:
    TestManager();
    ~TestManager() {}

public:
    static TestManager* instance();
    static void kill();

    bool execute();
    void addTest(Test* test);

protected:
    bool countTestsInFile(const std::string& filename, int& count) const;

private:
    std::list<Test*> tests_;
    static TestManager* singleton_;
};

/*******************************/

#endif // tests_h
