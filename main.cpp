#include <SDL2/SDL.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "main.h"
#include "game.hpp"

#ifndef MAIN
#define MAIN

const uint swidth = 1024;
const uint sheight = 512;
const char* window_name = "Physics CA";

SDL_Window* window;
SDL_Surface* screenSurface;
SDL_Renderer* renderer;

bool init() { //Initialize SDL
    
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return false;
    }
    else {
        //Create window
        window = SDL_CreateWindow( window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, swidth, sheight, SDL_WINDOW_SHOWN );
        
        if( window == NULL ) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            return false;
        } else {
            renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            
            if( renderer == NULL ) {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                return false;
            } else {
                //Initialize renderer color
                SDL_SetRenderDrawColor( renderer, 0x24, 0x17, 0x5a, 0xFF );

                int flags = IMG_INIT_PNG; //We only care about png here
                if ( !( IMG_Init(flags) & flags) ) {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    return false;
                } else {
                    //Get window surface
                    //screenSurface = SDL_GetWindowSurface( window );
                }
            }
        }
    }
    return true;
}

void close () {

    end();
    
    //Destroy window
    SDL_DestroyWindow( window );
    SDL_DestroyRenderer(renderer);
    window = NULL;
    renderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

int main( int argc, char* args[] ) {

    init();
    setup(renderer, swidth, sheight);
    
    
    bool quit = false;//Main loop flag
    
    SDL_Event e;//Event handler
    
    //Current time start time
    int startTime = 0;
    
    while (!quit) {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 ) {
            //User requests quit
            if( e.type == SDL_QUIT ) {
                quit = true;
            }
            
            event(e);
        }
        int temp = SDL_GetTicks();
        int dt = temp - startTime;
        startTime = temp;
        update(dt);
        draw(renderer, swidth, sheight);

    }
    close();
    
    return 0;
}

#endif
