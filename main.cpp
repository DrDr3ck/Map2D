#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
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
    std::ignore = argc;
    std::ignore = argv;
    // Check tests first
    TestManager* test_manager = TestManager::instance();
    if( !test_manager->execute() ) {
        return 1;
    }
    test_manager->kill();

    SDL_Init(SDL_INIT_VIDEO);

    // Create window
    int camera_width = Session::instance()->getInteger("*camera*width", 1000);
    int camera_height = Session::instance()->getInteger("*camera*height", 800);
    SDLCamera* camera = new SDLCamera(camera_width,camera_height);
    if( !camera->valid() ) {
        delete camera;
        return -1;
    }

    std::string language = Session::instance()->getString("*language", "none");
    if( language != "none" ) {
        language = "language/" + language + ".txt";
    }

    Translator::instance()->readDictionary(language);

    TextureMgr::instance()->loadAllItems(camera->main_renderer());

    TileSetLib* tileset = TileSetLib::instance();
    CharacterSetLib::instance()->init( camera->main_renderer() );

    PeopleGroup group;

    std::string filename("save01.arc");
    MapData data(10,6);
    JobMgr job_mgr(camera->main_renderer());

    GameBoard board(&group, &data, &job_mgr);

    //if( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1 ) {
    //    Logger::error() << "Cannot init sound because of " << Mix_GetError() << Logger::endl;
    //}

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
        if( group.group().size() == 1 ) {
            Position position = {1,1};
            Character* people = new Character("Bill", position, 5);
            people->setDirection(1,0);
            group.add(people);
        }
        // end test
    } else {
        Logger::warning() << tr("cannot find save named ") << filename << Logger::endl;
        // otherwise, create a random map
        MapData::createMap(&data);

        // with a robot
        Position position = {data.width()/2,data.height()/2};
        Character* people = new Character("B0b31", position, 0);
        people->setDirection(1,0);
        group.add(people);
    }

    MapView mapview(camera, &data, &group, &job_mgr);
    FontLib* font_manager = FontLib::instance();
    camera->init();

    // Main loop
    std::chrono::steady_clock::time_point start_clock = std::chrono::steady_clock::now();
    bool ending = false;

    //LoggerMgr::instance()->clear();

    const int FPS = 30;
    double delay_in_us = 1000000 / FPS;

    double microsecond = 0;
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
            if( !camera->isInPause() ) {
                board.animate(delay_clock_us/1000.);
            }
            start_clock = end_clock;
            microsecond += delay_clock_us;
            fps++;

        }

        if( microsecond > 1000000 ) {
            //Logger::debug() << "FPS: " << fps << Logger::endl;
            microsecond = 0;
            fps = 0;
        }

    }

    ArchiveConverter::save(&board, filename);

    tileset->kill();
    font_manager->kill();
    Session::instance()->kill();
    TextureMgr::instance()->kill();

    delete camera;

    Logger::debug() << "\n *** Game correctly ended ***" << Logger::endl;
    return 0;
}
