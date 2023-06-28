#include <SDL2/SDL.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

Uint32 getPixel( SDL_Surface * surface, uint x, uint y);
    
SDL_Surface* loadSurface( char* path);
    
SDL_Texture* loadTexture( char* path, bool colorKey);
