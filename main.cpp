#include <SDL2/SDL.h>
#include <iostream>
#include <cstdlib>

#include "map.h"
#include "sdl_camera.h"
#include "tests.h"

bool initSDL() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Cannot initialize SDL : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    return true;
}

int main(int /*argc*/, char** /*argv*/) {
    // Check tests first
    TestManager manager;
    if( !manager.execute() ) {
        return 1;
    }

    // Init of SDL
    if( !initSDL() ) {
        return -1;
    }

    // Create window
    SDLCamera* camera = new SDLCamera();
    if( !camera->valid() ) {
        delete camera;
        return -1;
    }

    MapData data(10,6);
    MapView* map_view = new MapView(&data);
    camera->addView(map_view);

    // Main loop
    bool ending = false;
    while(!ending) {
        camera->handleEvent();
        if( camera->quit() ) {
            ending = true;
        }
        camera->render();
    }

    delete camera;

    return 0;
}
