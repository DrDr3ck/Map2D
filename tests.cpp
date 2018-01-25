#include "tests.h"

#include "archive.h"
#include "font.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <random>
#include <SDL2/SDL_ttf.h>

#define CHECK(x, y) do { \
  bool retval = (x); \
  if (retval == false) { \
    fprintf(stderr, "\nRuntime error: %s returned %d at %s:%d", #x, retval, __FILE__, __LINE__); \
    y; \
  } \
} while (0)

#define CHECK_POINTER(x, y) do { \
  bool retval = (x != nullptr); \
  if (retval == false) { \
    fprintf(stderr, "\nRuntime error: %s returned %d at %s:%d", #x, retval, __FILE__, __LINE__); \
    y; \
  } \
} while (0)

#define CHECK_EQUAL(x1, x2, y) do { \
  bool retval = (x1 == x2); \
  if (retval == false) { \
    fprintf(stderr, "\nRuntime error: %s(%d) == %s(%d) returned %d at %s:%d", #x1, x1, #x2, x2, retval, __FILE__, __LINE__); \
    y; \
  } \
} while (0)

/*******************************************/

CheckingTest::CheckingTest() : Test("CheckingTest") {
}

bool CheckingTest::do_execute() {
   return true;
}

bool TileTest::do_execute() {
    Tile tile(10, Tile::BLOCK);
    CHECK_EQUAL(tile.id(), 10, return false;);
    CHECK_EQUAL(tile.type(), Tile::BLOCK, return false;);
    return true;
}

bool MapTest::do_execute() {
    MapData map_data(10,6);

    CHECK_EQUAL(map_data.width(), 10, return false;);
    CHECK_EQUAL(map_data.height(), 6, return false;);

    map_data.addWall(3,2);
    map_data.addWall(4,2);
    // check that we can add several time the same wall
    // without breaking data tile ids
    map_data.addWall(4,2);
    // add and remove wall
    map_data.addWall(5,2);
    Tile const_tile = map_data.tile(5,2);
    CHECK_EQUAL(const_tile.id(), 1, return false;);
    CHECK_EQUAL(const_tile.type(), Tile::WALL, return false;);
    map_data.removeWall(5,2);
    // check that we can add several time the same wall
    // without breaking data tile ids
    map_data.removeWall(5,2);
    map_data.removeWall(6,2);

    const_tile = map_data.tile(3,2);
    CHECK_EQUAL(const_tile.id(), 4, return false;);
    CHECK_EQUAL(const_tile.type(), Tile::WALL, return false;);

    const_tile = map_data.tile(4,2);
    CHECK_EQUAL(const_tile.id(), 1, return false;);
    CHECK_EQUAL(const_tile.type(), Tile::WALL, return false;);

    const_tile = map_data.tile(5,2);
    CHECK_EQUAL(const_tile.id(), 0, return false;);
    CHECK_EQUAL(const_tile.type(), Tile::EMPTY, return false;);

    // save/load
    std::string filename("test.save");
    MapDataConverter converter;
    converter.save(&map_data, filename);
    MapData map_data2(1,1);
    converter.load(&map_data2, filename);

    remove(filename.c_str());

    CHECK_EQUAL(map_data.width(), map_data2.width(), return false;);
    CHECK_EQUAL(map_data.height(), map_data2.height(), return false;);

    for( int i=0; i < map_data.width(); i++ ) {
        for( int j=0; j < map_data.height(); j++ ) {
            const Tile& map_tile1 = map_data.tile(i,j);
            const Tile& map_tile2 = map_data2.tile(i,j);
            CHECK_EQUAL(map_tile1.id(), map_tile2.id(), return false;);
            CHECK_EQUAL(map_tile1.type(), map_tile2.type(), return false;);
        }
    }

    return true;
}

bool FontTest::do_execute() {
    TTF_Init();

    TTF_Font* font16 = FontLib::instance()->getFont("pixel11", 16);
    CHECK_POINTER(font16, return false;);

    TTF_Font* font12 = FontLib::instance()->getFont("pixel11", 12);
    CHECK_POINTER(font12, return false;);

    // font16 should be different that font12
    CHECK_EQUAL( (font16==font12), false, return false; );

    return true;
}

/*******************************************/

TestManager::TestManager() {
    addTest(new CheckingTest());
    addTest(new TileTest());
    addTest(new MapTest());
    addTest(new FontTest());
}

TestManager* TestManager::instance() {
    if( singleton_ == nullptr ) {
        std::cout << "creating TestManager singleton" << std::endl;
        singleton_ =  new TestManager();
    }
    return singleton_;
}

void TestManager::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        std::cout << "destroying TestManager singleton" << std::endl;
        singleton_ = nullptr;
    }
}

bool TestManager::execute() {
    bool result = true;
    int nb_tests = 0;
    if( countTestsInFile("tests.h", nb_tests) ) {
        CHECK( int(tests_.size()) == nb_tests, return false;);
    } else {
        CHECK( tests_.size() > 0, return false;);
    }
    std::cout << "Executing " << tests_.size() << " tests ...\n";
    auto chrono_start = std::chrono::system_clock::now();
    for( auto test : tests_ ) {
        std::cout << "    > " << test->name();
        if( test->do_execute() ) {
            std::cout << " ... OK\n";
        } else {
            result = false;
            std::cout << " ... KO\n";
        }
    }
    auto chrono_end = std::chrono::system_clock::now();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono_end-chrono_start);
    std::cout << "Ending tests in " << ms.count() << " ms\n";
    return result;
}

bool TestManager::countTestsInFile(const std::string& filename, int& count) const {
    std::ifstream ifs( filename );
    if( !ifs.good() ) {
        return false;
    }
    std::string line;
    count = 0;
    while (std::getline(ifs, line)) {
        if( line.find("public Test") != std::string::npos ) {
            count++;
        }
    }
    return true;
}

void TestManager::addTest(Test* test) {
    tests_.push_back(test);
}

// Initialize singleton_ to nullptr
TestManager* TestManager::singleton_ = nullptr;
