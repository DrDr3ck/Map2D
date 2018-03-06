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

#include "tests.h"

int main(int /*argc*/, char** /*argv*/) {
    // Check tests first
    TestManager* test_manager = TestManager::instance();
    if( !test_manager->execute() ) {
        return 1;
    }
    test_manager->kill();

    // Create window
    SDLCamera* camera = new SDLCamera();
    if( !camera->valid() ) {
        delete camera;
        return -1;
    }

    TileSetLib* tileset = TileSetLib::instance();
    CharacterSetLib::instance()->init( camera->main_renderer() );

    PeopleGroup group;

    std::string filename("save01.arc");
    ArchiveConverter converter;
    MapData data(50,30);

    GameBoard board(&group, &data);

    // check if save already exists
    std::ifstream f(filename.c_str());
    if( f.good() ) {
        // if save exists, load it
        converter.load(&board, filename);

        // test
        if( group.group().size() == 0 ) {
            Position position = {2,1};
            Character* people = new Character("Bob", position, 0);
            people->setDirection(1,0);
            group.add(people);
        }
        // end test
    } else {
        // otherwise, create a random map (TODO)
        data.tile(2,2).setTile(8,Tile::BLOCK,Tile::NONE,Tile::METAL);

        // with a robot
        Position position = {2,1};
        Character* people = new Character("Bob", position, 0);
        people->setDirection(1,0);
        group.add(people);
    }
    MapView mapview(camera, &data, &group);

    FontLib* font_manager = FontLib::instance();

    // Main loop
    std::chrono::steady_clock::time_point start_draw = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point start_anim = std::chrono::steady_clock::now();
    bool ending = false;
    while(!ending) {

        camera->handleEvent();
        if( camera->quit() ) {
            ending = true;
        }
        std::chrono::steady_clock::time_point end_draw = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point end_anim = std::chrono::steady_clock::now();
        double delay_draw_us = std::chrono::duration_cast<std::chrono::microseconds>(end_draw - start_draw).count();
        double delay_anim_us = std::chrono::duration_cast<std::chrono::microseconds>(end_anim - start_anim).count();
        if( delay_draw_us > 25000 ) {
            camera->render(delay_draw_us);
            start_draw = end_draw;
        }
        if( delay_anim_us > 100000 ) {
            board.animate(delay_anim_us/1000.);
            start_anim = end_anim;
        }
        //SDL_Delay(25);
    }

    converter.save(&board, filename);

    tileset->kill();
    font_manager->kill();

    delete camera;

    return 0;
}
