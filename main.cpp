#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <chrono>

#include "map.h"
#include "character.h"
#include "font.h"
#include "archive.h"
#include "sdl_camera.h"
#include "logger.h"
#include "translator.h"
#include "session.h"
#include "texture_mgr.h"
#include "craft_mgr.h"

#include "tests.h"

int main(int argc, char** argv) {
    // Check tests first
    TestManager* test_manager = TestManager::instance();
    if( !test_manager->execute() ) {
        return 1;
    }
    test_manager->kill();

    // Create window
    SDLCamera* camera = new SDLCamera(800,600);
    if( !camera->valid() ) {
        delete camera;
        return -1;
    }

    std::string language = "none";
    if( argc == 3 ) {
        if( std::string(argv[1]) == "-language" ) {
            language = "language/";
            language.append(argv[2]);
            language.append(".txt");
        }
    }

    Translator::instance()->readDictionary("language/fr.txt");

    TextureMgr::instance()->loadAllItems(camera->main_renderer());

    TileSetLib* tileset = TileSetLib::instance();
    CharacterSetLib::instance()->init( camera->main_renderer() );

    PeopleGroup group;

    std::string filename("save01.arc");
    MapData data(100,60);
    JobMgr job_mgr(camera->main_renderer());

    GameBoard board(&group, &data, &job_mgr);

    // check if save already exists
    std::ifstream f(filename.c_str());
    if( f.good() ) {
        Logger::info() << tr("loading existing save") << Logger::endl;
        // if save exists, load it
        ArchiveConverter::load(&board, filename);

        // test
        if( group.group().size() == 0 ) {
            Position position = {2,1};
            Character* people = new Character("Bob", position, 0);
            people->setDirection(1,0);
            group.add(people);
        }
        // end test
    } else {
        Logger::warning() << tr("cannot find save named ") << filename << Logger::endl;
        // otherwise, create a random map
        MapData::createMap(&data);

        // with a robot
        Position position = {2,1};
        Character* people = new Character("B0b31", position, 0);
        people->setDirection(1,0);
        group.add(people);
    }
    MapView mapview(camera, &data, &group, &job_mgr);

    FontLib* font_manager = FontLib::instance();

    // Main loop
    std::chrono::steady_clock::time_point start_clock = std::chrono::steady_clock::now();
    bool ending = false;

    //LoggerMgr::instance()->clear();

    const int FPS = 30;
    double delay_in_us = 1000000 / FPS;

    double second = 0;
    int fps = 0;
    while(!ending) {

        camera->handleEvent();
        if( camera->quit() ) {
            ending = true;
        }

        std::chrono::steady_clock::time_point end_clock = std::chrono::steady_clock::now();
        double delay_clock_us = std::chrono::duration_cast<std::chrono::microseconds>(end_clock - start_clock).count();
        if( delay_clock_us > delay_in_us ) {
            camera->render(delay_clock_us/1000.);
            board.animate(delay_clock_us/1000.);
            start_clock = end_clock;
            second += delay_clock_us;
            fps++;

        }

        if( second > 1000000 ) {
            Logger::debug() << "FPS: " << fps << Logger::endl;
            second = 0;
            fps = 0;
        }

    }

    ArchiveConverter::save(&board, filename);

    tileset->kill();
    font_manager->kill();
    Session::instance()->kill();
    TextureMgr::instance()->kill();

    delete camera;

    return 0;
}
