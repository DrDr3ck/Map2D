#include "tests.h"

#include "archive.h"
#include "font.h"
#include "job.h"
#include "perlin_noise.h"
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
    CHECK_EQUAL(const_tile.type(), Tile::FLOOR, return false;);

    /*
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
    */
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

bool CharacterTest::do_execute() {
    Position position = {10,5};
    Character people("Bob", position, 0);

    CHECK_EQUAL( people.tilePosition().x, 10, return false; );
    CHECK_EQUAL( people.tilePosition().y, 5, return false; );
    return true;
}

bool ActionTest::do_execute() {
    Position position = {0,0};
    Character people("Bob", position, 0);
    CHECK_EQUAL( people.actionDescription().empty(), true, return false; );
    NoAction action(&people);
    CHECK_EQUAL( people.actionDescription().empty(), false, return false; );
    return true;
}

bool JobTest::do_execute() {
    JobMgr mgr(nullptr);
    int count = mgr.size();
    // no job
    CHECK_EQUAL( count, 0, return false; );

    // add a job
    Job job("test", {0,0}, "none", 1000);
    mgr.addJob(&job);
    count = mgr.size();
    CHECK_EQUAL( count, 1, return false; );

    // adding twice: should not adding it
    mgr.addJob(&job);
    count = mgr.size();
    CHECK_EQUAL( count, 1, return false; );

    // assign job to people
    Position position = {10,5};
    Character people("Bob", position, 0);
    Job* available_job = mgr.getFirstAvailableJob();
    CHECK_POINTER( available_job, return false; );
    available_job->takeJob(&people);

    available_job = mgr.getFirstAvailableJob();
    // no more available job
    CHECK_EQUAL( (available_job == nullptr), true, return false; );

    // check presence of "none.png"
    std::ifstream f("none.png");
    CHECK_EQUAL( f.good(), true, return false; );

    return true;
}

bool ChestTest::do_execute() {
    Chest chest(16);
    BasicItem stone("stone");
    int result = chest.addItem(stone, 3);
    result = chest.addItem(stone, 2);
    CHECK_EQUAL( result, 0, return false; );
    CHECK_EQUAL( chest.sizeAvailable(), 15, return false; );
    result = chest.removeItem(stone, 3);
    CHECK_EQUAL( result, 0, return false; );
    CHECK_EQUAL( chest.item(0).count(), 2, return false; );
    result = chest.removeItem(stone, 2);
    CHECK_EQUAL( result, 0, return false; );
    CHECK_EQUAL( chest.sizeAvailable(), 16, return false; );

    BasicItem sand("sand");
    chest.addItem(stone, 150);
    chest.addItem(sand, 50);
    CHECK_EQUAL( chest.item(0).count(), 99, return false; );
    CHECK_EQUAL( chest.item(1).count(), 51, return false; );
    CHECK_EQUAL( chest.item(2).count(), 50, return false; );
    CHECK_EQUAL( chest.sizeAvailable(), 13, return false; );

    result = chest.removeItem(sand, 60);
    CHECK_EQUAL( result, 10, return false; );
    CHECK_EQUAL( chest.sizeAvailable(), 14, return false; );
    return true;
}

namespace {
    // create and export surface
    void create_export_surface(unsigned int seed, float scale, int octaves, float persistance, float lacunarity, const std::string& filename, TerrainType* regions, int regionCount) {
        int size = 256;
        float** noise_map = Noise::generateNoiseMap(size, size, seed, scale, octaves, persistance, lacunarity);
        SDL_Surface* surface = SDL_CreateRGBSurface(0, size, size, 32, 0, 0, 0, 0);
        SDL_LockSurface(surface);
        Uint32* pixels = (Uint32*)surface->pixels;
        for( int y = 0; y < size; y++ ) {
            for( int x = 0; x < size; x++ ) {
                float value = noise_map[x][y];
                //std::cout << value << " ";
                int color = value * 255;
                int r = color;
                int g = color;
                int b = color;
                for( int region = 0; region < regionCount; region++ ) {
                    if( color < regions[region].height ) {
                        r = regions[region].red;
                        g = regions[region].green;
                        b = regions[region].blue;
                        break;
                    }
                }
                Uint32 couleur = SDL_MapRGB(surface->format, r, g, b);
                pixels[y * surface->w + x] = couleur;
            }
            //std::cout << std::endl;
        }
        SDL_UnlockSurface(surface);
        SDL_SaveBMP(surface, filename.data());
    }
}

bool PerlinTest::do_execute() {
    TerrainType terrain[6];
    terrain[0].label = "Sea";
    terrain[0].red = 30;
    terrain[0].green = 144;
    terrain[0].blue = 205;
    terrain[0].height = 50;
    terrain[1].label = "Sand";
    terrain[1].red = 238;
    terrain[1].green = 232;
    terrain[1].blue = 170;
    terrain[1].height = 55;
    terrain[2].label = "Light Forest";
    terrain[2].red = 0;
    terrain[2].green = 250;
    terrain[2].blue = 34;
    terrain[2].height = 100;
    terrain[3].label = "Forest";
    terrain[3].red = 34;
    terrain[3].green = 139;
    terrain[3].blue = 34;
    terrain[3].height = 150;
    terrain[4].label = "dark Forest";
    terrain[4].red = 0;
    terrain[4].green = 100;
    terrain[4].blue = 0;
    terrain[4].height = 200;
    terrain[5].label = "Snow";
    terrain[5].red = 200;
    terrain[5].green = 200;
    terrain[5].blue = 200;
    terrain[5].height = 220;

    unsigned int seed = 0;
    create_export_surface(seed, /*scale*/100,/*octaves*/4, /*persistance*/0.5, /*lacunarity*/2, "perlin_scale_100.bmp", terrain, 6);
    create_export_surface(seed, /*scale*/150,/*octaves*/4, /*persistance*/0.5, /*lacunarity*/2, "perlin_scale_150.bmp", terrain, 6);
    create_export_surface(seed, /*scale*/200,/*octaves*/4, /*persistance*/0.5, /*lacunarity*/2, "perlin_scale_200.bmp", terrain, 6);
    create_export_surface(seed, /*scale*/350,/*octaves*/4, /*persistance*/0.5, /*lacunarity*/2, "perlin_scale_350.bmp", terrain, 6);

    float scale = 150;
    create_export_surface(seed, scale,/*octaves*/3, /*persistance*/0.5, /*lacunarity*/2, "perlin_octave_3.bmp", terrain, 6);
    create_export_surface(seed, scale,/*octaves*/4, /*persistance*/0.5, /*lacunarity*/2, "perlin_octave_4.bmp", terrain, 6);
    create_export_surface(seed, scale,/*octaves*/5, /*persistance*/0.5, /*lacunarity*/2, "perlin_octave_5.bmp", terrain, 6);
    create_export_surface(seed, scale,/*octaves*/6, /*persistance*/0.5, /*lacunarity*/2, "perlin_octave_6.bmp", terrain, 6);

    int octaves = 4;
    create_export_surface(seed, scale, octaves, /*persistance*/0.3, /*lacunarity*/2, "perlin_persistance_3.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, /*persistance*/0.4, /*lacunarity*/2, "perlin_persistance_4.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, /*persistance*/0.5, /*lacunarity*/2, "perlin_persistance_5.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, /*persistance*/0.8, /*lacunarity*/2, "perlin_persistance_8.bmp", terrain, 6);

    float persistance = 0.5;
    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/1, "perlin_lacunarity_1.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/2, "perlin_lacunarity_2.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/3, "perlin_lacunarity_3.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/4, "perlin_lacunarity_4.bmp", terrain, 6);

    return true;
}

/*******************************************/

TestManager::TestManager() {
    addTest(new CheckingTest());
    addTest(new TileTest());
    addTest(new MapTest());
    addTest(new FontTest());
    addTest(new ActionTest());
    addTest(new CharacterTest());
    addTest(new JobTest());
    addTest(new ChestTest());
    addTest(new PerlinTest());
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
