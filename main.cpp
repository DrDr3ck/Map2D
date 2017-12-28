#include <SDL2/SDL.h>
#include <iostream>

#include "map.h"
#include "tests.h"

int main(int /*argc*/, char** /*argv*/) {
    // Check tests first
    TestManager manager;
    if( !manager.execute() ) {
        return 1;
    }

    // Our window
    SDL_Window* fenetre(0);
    SDL_Event evenements;


    // Init of SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Erreur lors de l'initialisation de la SDL : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create window
    fenetre = SDL_CreateWindow("Test SDL 2.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if(fenetre == 0) {
        std::cout << "Erreur lors de la creation de la fenetre : " << SDL_GetError() << std::endl;
        SDL_Quit();
       return -1;
    }

    MapData data(10,6);

    // Main loop
    bool terminer = false;
    while(!terminer) {
        SDL_WaitEvent(&evenements);

        if(evenements.window.event == SDL_WINDOWEVENT_CLOSE) {
            terminer = true;
        }
    }

    // Quit SDL
    SDL_DestroyWindow(fenetre);
    SDL_Quit();

    return 0;
}
