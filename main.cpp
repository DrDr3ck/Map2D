#include <SDL2/SDL.h>
#include <iostream>
#include <cstdlib>

#include "map.h"
#include "tests.h"

int main(int /*argc*/, char** /*argv*/) {
    // Check tests first
    TestManager manager;
    if( !manager.execute() ) {
        return 1;
    }

    // Init of SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Erreur lors de l'initialisation de la SDL : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Test SDL 2.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if(window == 0) {
        std::cout << "Erreur lors de la creation de la fenetre : " << SDL_GetError() << std::endl;
        SDL_Quit();
       return -1;
    }

    MapData data(10,6);

    SDL_Renderer* Main_Renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Surface* bg_surface = SDL_LoadBMP("background.bmp");
    SDL_Texture* background = SDL_CreateTextureFromSurface(Main_Renderer, bg_surface);

    // Main loop
    SDL_Event event;
    bool terminer = false;
    while(!terminer) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT) {
            terminer = true;
        }
        SDL_RenderCopy(Main_Renderer, background, NULL, NULL);
        SDL_RenderPresent(Main_Renderer);
        SDL_Delay(100);
    }

    // Quit SDL
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
