#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "map.h"
#include "archive.h"
#include "sdl_camera.h"
#include "tests.h"

int main(int /*argc*/, char** /*argv*/) {
    // Check tests first
    TestManager test_manager;
    if( !test_manager.execute() ) {
        return 1;
    }

    // Create window
    SDLCamera* camera = new SDLCamera();
    if( !camera->valid() ) {
        delete camera;
        return -1;
    }

    std::string filename("save01.arc");
    MapDataConverter converter;
    MapData data(10,6);

    // check if save already exists
    std::ifstream f(filename.c_str());
    if( f.good() ) {
        // if save exists, load it
        converter.load(&data, filename);
    } else {
        // otherwise, create a random map (TODO)
        data.tile(2,2).setTile(8,Tile::BLOCK);
    }
    MapView* map_view = new MapView(&data);
    camera->addView(map_view);

    TileSet* tileset = TileSet::instance();

    // Main loop
    bool ending = false;
    while(!ending) {
        camera->handleEvent();
        if( camera->quit() ) {
            ending = true;
        }
        camera->render();
    }

    converter.save(&data, filename);

    tileset->kill();

    delete camera;

    return 0;
}
