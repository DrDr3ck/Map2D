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
#include "sdl_background.h"
#include "session.h"
#include "texture_mgr.h"
#include "craft_mgr.h"

#include "tests.h"

int main(int argc, char** argv) {
    std::ignore = argc;
    std::ignore = argv;

    SDL_Init(SDL_INIT_VIDEO);

    // Create window
    int camera_width = Session::instance()->getInteger("*camera*width", 1000);
    int camera_height = Session::instance()->getInteger("*camera*height", 800);
    SDLCamera* sdl_camera = new SDLCamera(camera_width,camera_height);
    if( !sdl_camera->valid() ) {
        delete sdl_camera;
        return -1;
    }

    std::string language = Session::instance()->getString("*language", "none");
    if( language != "none" ) {
        language = "language/" + language + ".txt";
        Translator::instance()->readDictionary(language);
    }

    // Splash screen: loading...
    std::chrono::steady_clock::time_point start_load = std::chrono::steady_clock::now();
    LoadView* load = new LoadView(sdl_camera);
    sdl_camera->render(0);

    // Check tests first
    TestManager* test_manager = TestManager::instance();
    if( !test_manager->execute() ) {
        sdl_camera->removeView(load);
        delete load;
        delete sdl_camera;
        return 1;
    }
    test_manager->kill();

    TextureMgr::instance()->loadAllItems(sdl_camera->main_renderer());
    CharacterSetLib::instance()->init( sdl_camera->main_renderer() );
    //if( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1 ) {
    //    Logger::error() << "Cannot init sound because of " << Mix_GetError() << Logger::endl;
    //}

    SDL_Cursor* wait_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    SDL_SetCursor(wait_cursor);

    int minimum_time = Session::instance()->getInteger("*splash_screen*minimum_time", 1);

    double delay_clock_us = 0;
    while( delay_clock_us < minimum_time*1000000 ) {
        std::chrono::steady_clock::time_point end_load = std::chrono::steady_clock::now();
        delay_clock_us = std::chrono::duration_cast<std::chrono::microseconds>(end_load - start_load).count();
    }

    SDL_FreeCursor(wait_cursor);


    SDL_Cursor* hand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_SetCursor(hand_cursor);

    // Select a game
    bool ending = false;
    bool game_selected = false;
    load->initButtons();
    while( !game_selected && !ending ) {
        sdl_camera->handleEvent();
        if( sdl_camera->quit() ) {
            ending = true;
        }
        sdl_camera->render(0);
        if( load->gameSelected() ) {
            game_selected = true;
        }
    }
    SDL_FreeCursor(hand_cursor);
    if( !ending ) {
        // init map
        PeopleGroup group;
        MapData data(32,40);
        JobMgr job_mgr(sdl_camera->main_renderer());
        GameBoard board(&group, &data, &job_mgr);

        // check if save already exists
        std::string filename = load->fileSelected();
        std::ifstream f(filename.c_str());
        if( f.good() ) {
            // if save exists, load it
            if( !ArchiveConverter::load(&board, filename) ) {
                return -1;
            }

            // test
            if( group.group().size() == 0 ) {
                Position position = {2,1};
                Character* people = new Character("Bob", position, 0);
                people->setDirection(1,0);
                group.add(people);
            }
            if( group.group().size() == 1 ) {
                Position position = {1,1};
                Character* people = new Character("Bill", position, 4);
                people->setDirection(1,0);
                group.add(people);
            }
            // end test
        } else {
            Logger::info() << tr("Creating a new game named ") << filename << Logger::endl;
            // otherwise, create a random map
            MapData::createMap(&data);

            // with 2 robots
            int tile_x, tile_y;
            data.getEmptyGrassTilePosition(tile_x, tile_y);
            Position position = {tile_x,tile_y};
            Character* people = new Character("B0b31", position, 0);
            people->setDirection(1,0);
            group.add(people);
            data.getEmptyGrassTilePosition(tile_x, tile_y);
            position = {tile_x,tile_y};
            people = new Character("B1ll03", position, 4);
            people->setDirection(1,0);
            group.add(people);
        }
        Session::instance()->setString("*save*filename", filename);

        MapView mapview(sdl_camera, &data, &group, &job_mgr);

        // initialize command center with items of all chests
        CommandCenter* cc = nullptr;
        std::vector<Chest*> chests;
        for( auto object : data.objects() ) {
            Chest* chest = dynamic_cast<Chest*>(object);
            if( chest != nullptr ) {
                chests.push_back(chest);
                continue;
            }
            if( cc == nullptr ) {
                cc = dynamic_cast<CommandCenter*>(object);
            }
        }
        if( cc == nullptr ) {
            int tile_x, tile_y;
            data.getEmptyGrassTilePosition(tile_x, tile_y);
            cc = new CommandCenter();
            data.addObject(cc, tile_x, tile_y);
        }
        if( cc != nullptr && chests.size() > 0 ) {
            cc->reset();
            CommandCenter::init(cc, chests);
        }

        sdl_camera->addView(&mapview);
        sdl_camera->removeView(load);
        delete load;
        load = nullptr;

        // automatic save
        std::chrono::steady_clock::time_point automatic_save_clock = std::chrono::steady_clock::now();

        std::chrono::steady_clock::time_point start_clock = std::chrono::steady_clock::now();

        const int FPS = 30;
        double delay_in_us = 1000000 / FPS;

        double microsecond = 0;
        int fps = 0;

        sdl_camera->initManager();
        Logger::info() << tr("Game loaded") << Logger::endl;
        // Main loop
        while(!ending) {

            sdl_camera->handleEvent();
            if( sdl_camera->quit() ) {
                ArchiveConverter::save(&board, filename);
                Session::instance()->kill();
                ending = true;
            }

            std::chrono::steady_clock::time_point end_clock = std::chrono::steady_clock::now();
            double delay_clock_us = std::chrono::duration_cast<std::chrono::microseconds>(end_clock - start_clock).count();
            double speed_delay_clock_us = delay_clock_us * sdl_camera->speedTime();
            if( speed_delay_clock_us > delay_in_us * sdl_camera->speedTime() ) {
                sdl_camera->render(speed_delay_clock_us/1000.);
                if( !sdl_camera->isInPause() ) {
                    board.animate(speed_delay_clock_us/1000.);
                }
                start_clock = end_clock;
                microsecond += delay_clock_us;
                fps++;

            }

            double automatic_clock_s = std::chrono::duration_cast<std::chrono::microseconds>(end_clock - automatic_save_clock).count() / 1000000.f;
            int automatic_save_delay = Session::instance()->getInteger("*save*automatic", 10);
            if( automatic_clock_s > automatic_save_delay*60 ) {
                // save automatically
                ArchiveConverter::save(&board, filename);
                // reset time for next automatic save
                automatic_save_clock = std::chrono::steady_clock::now();
            }

            if( microsecond > 1000000 ) {
                if( Session::instance()->getBoolean("*display_F3", true) ) {
                    Logger::debug() << "FPS: " << fps << Logger::endl;
                }
                microsecond = 0;
                fps = 0;
            }

        }
    }

    TileSetLib::instance()->kill();
    FontLib::instance()->kill();
    TextureMgr::instance()->kill();

    if( load != nullptr ) {
        delete load;
    }

    delete sdl_camera;

    Logger::debug() << "\n *** Game correctly ended ***" << Logger::endl;
    return 0;
}
