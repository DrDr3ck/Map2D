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

    std::string filename("save01.arc");
    MapDataConverter converter;
    MapData data(50,30);

    // check if save already exists
    std::ifstream f(filename.c_str());
    if( f.good() ) {
        // if save exists, load it
        converter.load(&data, filename);
    } else {
        // otherwise, create a random map (TODO)
        data.tile(2,2).setTile(8,Tile::BLOCK,Tile::NONE,Tile::METAL);
    }
    MapView* map_view = new MapView(&data);
    camera->setMapView(map_view);

    FontLib* font_manager = FontLib::instance();

    // Main loop
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    bool ending = false;
    while(!ending) {

        camera->handleEvent();
        if( camera->quit() ) {
            ending = true;
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double delay = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        camera->render(delay);
        start = end;
        //SDL_Delay(25);
    }

    converter.save(&data, filename);

    tileset->kill();
    font_manager->kill();

    delete camera;

    return 0;
}
