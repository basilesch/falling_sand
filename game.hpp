#include <SDL2/SDL.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL2_gfxPrimitives.h>

#include "PhysicsCA.hpp"

#ifndef GAME_HPP
#define GAME_HPP

#define ratio 4
cell_grid * grid = new cell_grid(ratio*64, (64*ratio)/2);
//ratio * 64 * grid_scale = 1024
int brush_type = PIXEL_TYPE_SAND;
int brush_size = 3;
bool circle_brush = true;
float grid_scale = (1024/(64*ratio));


// ------------------------------------------------------------------------------------------

bool KEYS[SDL_NUM_SCANCODES];  // 322 is the number of SDLK_DOWN events
bool mouseIsPressed = false;

void event (SDL_Event e) {
    /*Select things
    switch( e.key.keysym.sym ) {
        default:
        break;
    }*/
    switch (e.type) {
        case SDL_KEYUP :
            if (e.key.keysym.sym < SDL_NUM_SCANCODES) KEYS[e.key.keysym.sym] = false;
            break;
        //User presses a key
        case SDL_KEYDOWN :
            if (e.key.keysym.sym < SDL_NUM_SCANCODES) KEYS[e.key.keysym.sym] = true;
            switch (e.key.keysym.sym) {
                case SDLK_SPACE:
                    circle_brush = !circle_brush;
                    std::cout << "Brush shape : " << (circle_brush ? "circle" : "square") << " \n";
                    break;
                case SDLK_DOWN:
                    brush_size --;
                    brush_size = (brush_size < 1) ? 1 : brush_size;
                    std::cout << "Brush size : " << brush_size << " \n";
                    break;
                case SDLK_UP:
                    brush_size ++;
                    std::cout << "Brush size : " << brush_size << " \n";
                    break;
                case SDLK_a:
                    brush_type --;
                    brush_type = (brush_type < 1) ? 1 : brush_type;
                    //if (brush_type == PIXEL_TYPE_BORDER) brush_type --;
                    std::cout << "Brush type : " << pixelTypes[brush_type].name << " \n";
                    break;
                case SDLK_z:
                    brush_type ++;                    
                    brush_type = (brush_type >= number_of_types) ? (number_of_types-1) : brush_type;
                    //if (brush_type == PIXEL_TYPE_BORDER) brush_type ++;
                    std::cout << "Brush type : " << pixelTypes[brush_type].name << " \n";
                    break;
                case SDLK_c:
                    grid->allocate();
                    break;
            }
            break;
            
            
        case SDL_MOUSEBUTTONDOWN :
            mouseIsPressed = true;
            break;
        case SDL_MOUSEBUTTONUP :
            mouseIsPressed = false;
            break;
        default :
            break;
    }
}

// ------------------------------------------------------------------------------------------

void draw_rect(SDL_Renderer * renderer, int x, int y, int w, int h, int r, int g, int b) {
    SDL_SetRenderDrawColor( renderer, r, g, b, 0xFF );
    SDL_Rect destination = {x, y, w, h};
    SDL_RenderFillRect(renderer, &destination);
}

void draw(cell_grid * c, SDL_Renderer * renderer, int o_x, int o_y, int scale) {
    for(uint y = 0; y < c->h; y++) {
        for(uint x = 0; x < c->w; x++) {
            int color = c->get(y, x).color; //pixelTypes[c->get(y, x).type].color1;
            int r = ((color & 0xff000000)>> 24);
            int g = ((color & 0x00ff0000)>> 16);
            int b = ((color & 0x0000ff00)>> 8);
            int a = (color & 0x000000ff);
            //std::cout << color << "  ;  " << "r : " << r << " ; g : " << g << " ; b : " << b << " \n";
            if (a != 0) draw_rect(renderer, o_x + scale * x, o_y + scale * y, scale, scale, r, g, b);
        }
    }
}

void setup (SDL_Renderer * renderer, uint sWidth, uint sHeight) {
    std::cout << "---------------------------------------------------------------------------- \n";
    std::cout << "Use a or z to change brush type, up or down to change brush size. \n"
              << "Space changes shape between circle and square, c clear the screen. enjoy \n";
    std::cout << "List of available elements : \n";
    for (int i = 0; i < number_of_types; i++) {
        if (i != PIXEL_TYPE_BORDER) std::cout << pixelTypes[i].name << " ; ";
    }
    std::cout << "\n";
    std::cout << "---------------------------------------------------------------------------- \n\n";
    for(int i = 0; i < SDL_NUM_SCANCODES; i++) { // init them all to false
        KEYS[i] = false;
    }
    
    grid->draw_circle(PIXEL_TYPE_WATER, (90 *grid->w)/256, (11*grid->h)/128, ratio*7/4);
    grid->draw_circle(PIXEL_TYPE_SAND , (105*grid->w)/256, (67*grid->h)/128, ratio*9/4);
    grid->draw_circle(PIXEL_TYPE_SAND , (73 *grid->w)/256, (52*grid->h)/128, ratio*13/4);
    
    grid->draw_circle(PIXEL_TYPE_ROCK , (91 *grid->w)/256, (77*grid->h)/128, ratio*5/4);
}

int dt_bucket = 0;
int dt_threshold = 1000/60;
void update (int dt) {//dt in milliseconds
    dt_bucket+= dt;
    if ( dt_bucket > dt_threshold ) {
        grid->step();
        dt_bucket -= dt_threshold;
    }
    if (mouseIsPressed) {
        int x, y;
        SDL_GetMouseState( &x, &y );
        if (circle_brush)   grid->draw_circle(brush_type, (x-0)/grid_scale, (y-0)/grid_scale, brush_size);
        else                grid->draw_square(brush_type, (x-0)/grid_scale, (y-0)/grid_scale, brush_size);
    }
}

void draw(SDL_Renderer * renderer, uint sWidth, uint sHeight) {
    SDL_RenderClear(renderer); //clear
    
    /*SDL_RenderCopy(renderer, spriteSheet, NULL, &destination);
    
    SDL_SetRenderDrawColor( renderer, 0xEF, 0x10, 0x20, 0xFF ); //red
    SDL_Rect destination = {100, 300, 50, 70};
    SDL_RenderFillRect(renderer, &destination);
    
    draw_rect(renderer, 300, 300, 50, 70, 0x00, 0xF0, 0x10); */
    
    draw(grid, renderer, 0, 0, grid_scale); 
    
    //SDL_SetRenderDrawColor( renderer, 0x24, 0x17, 0x5a, 0xFF ); //fill background in blurple
    SDL_SetRenderDrawColor( renderer, 0x03, 0x03, 0x03, 0xFF );
    SDL_RenderPresent(renderer); //update 
}
void end() {
 
}


#endif
