#include <SDL2/SDL.h>
#include <iostream>
#include <cstdlib>

#include "map.h"
#include "camera.h"
#include "tests.h"

int main(int /*argc*/, char** /*argv*/) {
    // Check tests first
    TestManager manager;
    if( !manager.execute() ) {
        return 1;
    }

    // Init of SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Cannot initialize SDL : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create window
    Camera* camera = new Camera();
    SDL_Window* window = camera->window();
    if(window == nullptr) {
        std::cout << "Cannot create window : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    MapData data(10,6);
    MapView* map_view = new MapView(&data);
    camera->addView(map_view);

    // Main loop
    SDL_Event event;
    bool terminer = false;
    while(!terminer) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT) {
            terminer = true;
        } else {
            camera->handleEvent(event);
            if( camera->quit() ) {
                terminer = true;
            }
        }
        camera->render();
    }

    delete camera;
    SDL_Quit();

    return 0;
}
