#include "tests.h"

#include "archive.h"
#include "font.h"
#include "job.h"
#include "perlin_noise.h"
#include "xml_document.h"
#include "sdl_background.h"
#include "logger.h"
#include "action.h"
#include "session.h"
#include "translator.h"
#include "craft_mgr.h"
#include <stdio.h>
#include <cstdlib>
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

#define CHECK_STR_EQUAL(str1, str2, y) do { \
  bool retval = (str1.compare(str2) == 0); \
  if (retval == false) { \
    fprintf(stderr, "\nRuntime error: %s(%s) == %s(%s) returned %d at %s:%d", #str1, str1.data(), #str2, str2.data(), retval, __FILE__, __LINE__); \
    y; \
  } \
} while (0)

/*******************************************/

Test::Test(std::string name) : test_name_(name) {
}

/*******************************************/

CheckingTest::CheckingTest() : Test("CheckingTest") {
}

bool CheckingTest::do_execute() {
   return true;
}

bool UtilityTest::do_execute() {
    std::string original = "test $1 test";
    Utility::replace(original, "$1", "test");
    CHECK_STR_EQUAL(original, std::string("test test test"), return false;);

    original = "$1 test $2";
    Utility::replace(original, "$1", "test");
    Utility::replace(original, "$2", "test");
    CHECK_STR_EQUAL(original, std::string("test test test"), return false;);

    std::string message = "Missing $1 element(s) of $2 to craft $3";
    int nb = 5;
    Utility::replace(message, "$1", Utility::itos(nb));
    CountedItem counted_item(BasicItem("sand"), 5);
    Utility::replace(message, "$2", counted_item.item().name());
    Craft craft("iron chest", Craft::CraftType::OBJECT);
    Utility::replace(message, "$3", craft.name());
    CHECK_STR_EQUAL(message, std::string("Missing 5 element(s) of sand to craft iron chest"), return false;);

    bool result = Utility::endsWith("engine", "furnace");
    CHECK_EQUAL(result, false, return false;);
    result = Utility::endsWith("stone_furnace", "furnace");
    CHECK_EQUAL(result, true, return false;);

    return true;
}

bool TileTest::do_execute() {
    Tile tile(10, Tile::BLOCK);
    CHECK_EQUAL(tile.id(), 10, return false;);
    CHECK_EQUAL(tile.cell_type(), Tile::BLOCK, return false;);
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
    CHECK_EQUAL(const_tile.cell_type(), Tile::WALL, return false;);
    map_data.removeWall(5,2);
    // check that we can add several time the same wall
    // without breaking data tile ids
    map_data.removeWall(5,2);
    map_data.removeWall(6,2);

    const_tile = map_data.tile(3,2);
    CHECK_EQUAL(const_tile.id(), 4, return false;);
    CHECK_EQUAL(const_tile.cell_type(), Tile::WALL, return false;);

    const_tile = map_data.tile(4,2);
    CHECK_EQUAL(const_tile.id(), 1, return false;);
    CHECK_EQUAL(const_tile.cell_type(), Tile::WALL, return false;);

    const_tile = map_data.tile(5,2);
    CHECK_EQUAL(const_tile.id(), 0, return false;);
    CHECK_EQUAL(const_tile.cell_type(), Tile::FLOOR, return false;);

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
            CHECK_EQUAL(map_tile1.cell_type(), map_tile2.cell_type(), return false;);
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

bool ChestFilterTest::do_execute() {
    ChestFilter filter;
    filter.addFilter("sand");
    CHECK_EQUAL(filter.isFiltered("sand"), true, return false;);
    CHECK_EQUAL(filter.isFiltered("stone"), false, return false;);
    filter.removeFilter("sand");
    CHECK_EQUAL(filter.isFiltered("sand"), false, return false;);
    filter.removeFilter("stone"); // nothing should happen

    return true;
}

bool CommandCenterTest::do_execute() {
    Chest chest(16);
    BasicItem stone("stone");
    int nb_total = 5;
    chest.addItem(stone, nb_total);
    std::vector<Chest*> chests;
    chests.push_back(&chest);
    CommandCenter* cc = new CommandCenter();
    CommandCenter::init(cc, chests);

    int stored_item = cc->countedItems(stone);
    CHECK_EQUAL( stored_item, nb_total, return false; );

    int nb_retract = 2;
    chest.removeItem(stone, nb_retract);
    stored_item = cc->countedItems(stone);
    CHECK_EQUAL( stored_item, nb_total-nb_retract, return false; );

    CommandCenter::destroy();
    delete cc;

    return true;
}

/*******************************************/

bool SessionTest::do_execute() {
    Session* instance = Session::instance();
    // set 'test' to true
    instance->setBoolean("*test", true);
    bool res1 = instance->getBoolean("*test");
    // check result, it should be true
    CHECK_EQUAL( res1, true, return false; );
    bool res2 = instance->getBoolean("*test", false);
    CHECK_EQUAL( res2, true, return false; );
    // get value of unknown 'second_test' that should be 'false' by default
    bool res3 = instance->getBoolean("*second_test", false);
    CHECK_EQUAL( res3, false, return false; );
    Session::kill(false); // do not save these values
    return true;
}

/*******************************************/

namespace {
    // create and export surface
    void create_export_surface(
        unsigned int seed, float scale, int octaves, float persistance, float lacunarity,
        const std::string& filename, TerrainType* regions, int regionCount
    ) {
        int sizeX = 50;
        int sizeY = 30;
        float** noise_map = Noise::generateNoiseMap(sizeX, sizeY, seed, scale, octaves, persistance, lacunarity);
        SDL_Surface* surface = SDL_CreateRGBSurface(0, sizeX, sizeY, 32, 0, 0, 0, 0);
        SDL_LockSurface(surface);
        Uint32* pixels = (Uint32*)surface->pixels;
        for( int y = 0; y < sizeY; y++ ) {
            for( int x = 0; x < sizeX; x++ ) {
                float value = noise_map[x][y];
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
        }
        SDL_UnlockSurface(surface);
        SDL_SaveBMP(surface, filename.data());
    }
}

bool PerlinTest::do_execute() {
    if( true ) {
        return true;
    }
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
    int octaves = 4;
    float persistance = 0.5f;
    float lacunarity = 2.f;
    create_export_surface(seed, /*scale*/100, octaves, persistance, lacunarity, "perlin_scale_100.bmp", terrain, 6);
    create_export_surface(seed, /*scale*/150, octaves, persistance, lacunarity, "perlin_scale_150.bmp", terrain, 6);
    create_export_surface(seed, /*scale*/200, octaves, persistance, lacunarity, "perlin_scale_200.bmp", terrain, 6);
    create_export_surface(seed, /*scale*/350, octaves, persistance, lacunarity, "perlin_scale_350.bmp", terrain, 6);

    float scale = 150;
    create_export_surface(seed, scale,/*octaves*/3, persistance, lacunarity, "perlin_octave_3.bmp", terrain, 6);
    create_export_surface(seed, scale,/*octaves*/4, persistance, lacunarity, "perlin_octave_4.bmp", terrain, 6);
    create_export_surface(seed, scale,/*octaves*/5, persistance, lacunarity, "perlin_octave_5.bmp", terrain, 6);
    create_export_surface(seed, scale,/*octaves*/6, persistance, lacunarity, "perlin_octave_6.bmp", terrain, 6);

    create_export_surface(seed, scale, octaves, /*persistance*/0.3, lacunarity, "perlin_persistance_3.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, /*persistance*/0.4, lacunarity, "perlin_persistance_4.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, /*persistance*/0.5, lacunarity, "perlin_persistance_5.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, /*persistance*/0.8, lacunarity, "perlin_persistance_8.bmp", terrain, 6);

    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/1, "perlin_lacunarity_1.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/2, "perlin_lacunarity_2.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/3, "perlin_lacunarity_3.bmp", terrain, 6);
    create_export_surface(seed, scale, octaves, persistance, /*lacunarity*/4, "perlin_lacunarity_4.bmp", terrain, 6);

    seed = rand();
    std::string filename = "perlin_random_";
    filename.append(Utility::itos(seed));
    filename.append(".bmp");
    create_export_surface(seed, scale, octaves, persistance, lacunarity, filename, terrain, 6);

    return true;
}

bool XMLTest::do_execute() {
    XMLNode* parent = new XMLNode("biomes");
    XMLNode* sea_biome = new XMLNode("biome", parent);
    sea_biome->addAttr(new XMLAttr("name", "Sea"));
    XMLNode* water_node = new XMLNode("water", sea_biome);
    water_node->addValuedAttr("min", "35%");
    water_node->addValuedAttr("max", "85%");
    XMLNode* rock_node = new XMLNode("rock", sea_biome);
    rock_node->addValuedAttr("min", "0%");
    rock_node->addValuedAttr("max", "5%");

    std::string filename("biomes_test.xml");
    XMLDocument::write_doc(parent, filename);

    XMLNode* result = XMLDocument::read_doc(filename);
    CHECK_POINTER(result, return false;);
    CHECK_STR_EQUAL( result->name(), std::string("biomes"), return false; );
    CHECK_EQUAL( result->attrCount(), 0, return false;);
    CHECK_EQUAL( result->nodeCount(), 1, return false;);

    sea_biome = result->node(0);
    CHECK_POINTER(sea_biome, return false;);
    CHECK_STR_EQUAL( sea_biome->name(), std::string("biome"), return false; );
    CHECK_EQUAL( sea_biome->attrCount(), 1, return false;);
    CHECK_EQUAL( sea_biome->nodeCount(), 2, return false;);
    CHECK_STR_EQUAL( sea_biome->attr(0)->label(), std::string("name"), return false; );
    CHECK_STR_EQUAL( sea_biome->attr(0)->value(), std::string("Sea"), return false; );

    water_node = sea_biome->node(0);
    CHECK_POINTER(water_node, return false;);
    CHECK_EQUAL( water_node->attrCount(), 2, return false;);
    CHECK_STR_EQUAL( water_node->name(), std::string("water"), return false; );
    CHECK_STR_EQUAL( water_node->attr(0)->label(), std::string("min"), return false; );
    CHECK_STR_EQUAL( water_node->attr(0)->value(), std::string("35%"), return false; );
    CHECK_STR_EQUAL( water_node->attr(1)->label(), std::string("max"), return false; );
    CHECK_STR_EQUAL( water_node->attr(1)->value(), std::string("85%"), return false; );
    CHECK_EQUAL( water_node->nodeCount(), 0, return false;);

    rock_node = sea_biome->node(1);
    CHECK_POINTER(rock_node, return false;);
    CHECK_EQUAL( rock_node->attrCount(), 2, return false;);
    CHECK_STR_EQUAL( rock_node->name(), std::string("rock"), return false; );
    CHECK_STR_EQUAL( rock_node->attr(0)->label(), std::string("min"), return false; );
    CHECK_STR_EQUAL( rock_node->attr(0)->value(), std::string("0%"), return false; );
    CHECK_STR_EQUAL( rock_node->attr(1)->label(), std::string("max"), return false; );
    CHECK_STR_EQUAL( rock_node->attr(1)->value(), std::string("5%"), return false; );
    CHECK_EQUAL( rock_node->nodeCount(), 0, return false;);

    //  delete file biomes_test.xml
    std::remove(filename.c_str());

    return true;
}

bool BiomeTest::do_execute() {
    Biome biome("forest");
    CHECK( biome.isValid(), return false;);
    CHECK_EQUAL( biome.heights().size(), 8, return false;);
    return true;
}

bool TranslatorTest::do_execute() {
    Translator::instance()->readDictionary("language/fr.txt");
    const std::string& result = tr("this is a test");
    CHECK_STR_EQUAL( result, std::string("ceci est un test"), return false; );
    return true;
}

bool CraftMgrTest::do_execute() {
    CraftMgr::instance()->loadCrafts("craft.xml");
    std::vector<Craft*> crafts = CraftMgr::instance()->craftsForMachine("workbench");
    CHECK( crafts.size() > 0, return false;);
    return true;
}

/*******************************************/

TestManager::TestManager() {
    addTest(new CheckingTest());
    addTest(new UtilityTest());
    addTest(new TileTest());
    addTest(new MapTest());
    addTest(new FontTest());
    addTest(new ActionTest());
    addTest(new CharacterTest());
    addTest(new JobTest());
    addTest(new ChestTest());
    addTest(new ChestFilterTest());
    addTest(new CommandCenterTest());
    addTest(new SessionTest());
    addTest(new PerlinTest());
    addTest(new XMLTest());
    addTest(new BiomeTest());
    addTest(new TranslatorTest());
    addTest(new CraftMgrTest());
}

TestManager* TestManager::instance() {
    if( singleton_ == nullptr ) {
        Logger::debug() << "creating TestManager singleton" << Logger::endl;
        singleton_ =  new TestManager();
    }
    return singleton_;
}

void TestManager::kill() {
    if( singleton_ != nullptr ) {
        delete singleton_;
        Logger::debug() << "destroying TestManager singleton" << Logger::endl;
        singleton_ = nullptr;
    }
}

bool TestManager::execute() {
    bool result = true;
    int nb_tests = 0;
    if( countTestsInFile("tests.h", nb_tests) ) {
        // if false, may be you forget to add a test in TestManager constructor ?
        CHECK( int(tests_.size()) == nb_tests, return false;);
    } else {
        CHECK( tests_.size() > 0, return false;);
    }
    Logger::debug() << "Executing " << tests_.size() << " tests ..." << Logger::endl;
    auto chrono_start = std::chrono::system_clock::now();
    for( auto test : tests_ ) {
        Logger debug = Logger::debug();
        debug << "    > " << test->name();
        if( test->do_execute() ) {
            debug << " ... OK" << Logger::endl;
        } else {
            result = false;
            debug << " ... KO" << Logger::endl;
        }
    }
    auto chrono_end = std::chrono::system_clock::now();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono_end-chrono_start);
    Logger::debug() << "Ending tests in " << ms.count() << " ms" << Logger::endl;
    return result;
}

/*!
 * Counts the number of classes in \p filename that derives from Test.
 */
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
    Logger::debug() << "add test " << test->name() << Logger::endl;
    tests_.push_back(test);
}

// Initialize singleton_ to nullptr
TestManager* TestManager::singleton_ = nullptr;
