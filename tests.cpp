#include "tests.h"

#include "archive.h"
#include <stdio.h>
#include <iostream>
#include <chrono>

#define CHECK(x, y) do { \
  bool retval = (x); \
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

    Tile& tile = map_data.tile(3,2);
    tile.setTile(2,Tile::DOOR);

    const Tile& const_tile = map_data.tile(3,2);
    CHECK_EQUAL(const_tile.id(), 2, return false;);
    CHECK_EQUAL(const_tile.type(), Tile::DOOR, return false;);

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

/*******************************************/

TestManager::TestManager() {
    addTest(new CheckingTest());
    addTest(new TileTest());
    addTest(new MapTest());
}

bool TestManager::execute() {
    bool result = true;
    CHECK( tests_.size() > 0, return false;);
    std::cout << "Executing " << tests_.size() << " tests ...\n";
    auto start = std::chrono::system_clock::now();
    for( auto test : tests_ ) {
        std::cout << "    > " << test->name();
        if( test->do_execute() ) {
            std::cout << " ... OK\n";
        } else {
            result = false;
            std::cout << " ... KO\n";
        }
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << "Ending tests in " << ms.count() << " ms\n";
    return result;
}

void TestManager::addTest(Test* test) {
    tests_.push_back(test);
}
