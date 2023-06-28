#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vec2f.hpp"
#include "rng.hpp"

#ifndef PHY_CA_HPP
#define PHY_CA_HPP

enum PIXEL_FLAG {
    PIXEL_FLAG_SKIP =                   0,    //here for the sake of completedness - does it need to be processed or can it be ignored ?
    PIXEL_FLAG_PHYSICAL =               1<<0, //is the movement simulated physicaly ?
    PIXEL_FLAG_DIAGONAL_CHECK =         1<<1, //should we check diagonals when physical movement is prevented ?
    PIXEL_FLAG_FLUID =                 1<<2, // -> static dispertion ; should we make a vertical wave if we can't go down ?
    PIXEL_FLAG_VMC =                    1<<3, //vertical movement conservation, ie "x to y splat"
    PIXEL_FLAG_IMMOVABLE =              1<<4, //should it stay in place constantly ?
    PIXEL_FLAG_FUNKY =               1<<5, //should it do a little dance ?
};

struct pixelTypes_s {
    char* name;                     //for potential UI or console logs
    float friction;                 //
    int dispersion_rate;            //to make fluids more... fluids
    float inertialResistance;       //once it stops moving, it stays in place -> probability to not change behaviour when interacted with.
    float relative_density;         //d = ρ / ρ_water, this quantity does not change with temperature
    //useful to get gravity acceleration, which depends entirely on this + mass, volume, pressure depending on you parameters. More versatile
    float max_speed;                //TODO : in absolutes, could be computed with air resistance/friction and density (which gives its weight)
    long flags;                     //
    int color1;                     //70%
    int color2;                     //25%
    int color3;                     //5%
};
typedef struct pixelTypes_s pixelType;

struct cell_s {
    int type;
    vec2f vel;
    vec2f precision_bucket; //retain "unspent" x distance to allow slow movements - not as useful for y movements ?
    bool isActive; //maybe should be changed, but allow inertia (stop moving and stays) and to stop updates (optimisation)
    int color;
    bool clock;
    bool movedYLastFrame;
};
typedef struct cell_s cell;


#define number_of_types 11

enum PIXEL_TYPE {
    PIXEL_TYPE_BORDER= 0,
    PIXEL_TYPE_AIR   = 1,
    PIXEL_TYPE_ROCK  = 2,
    PIXEL_TYPE_WATER = 3,
    PIXEL_TYPE_SAND  = 4,
    PIXEL_TYPE_DIRT  = 5,
    PIXEL_TYPE_COAL  = 6,
    PIXEL_TYPE_SAWDUST=7,
    PIXEL_TYPE_SMOKE  =8,
    PIXEL_TYPE_FUNKY  =9,
    PIXEL_TYPE_MERCURY=10,
};

//TODO : actually find a more elegant solution, such as mdesk, to organise data. Maybe some lua thingy ?

pixelType pixelTypes[number_of_types] = {
//      name    friction    dispersion  inertial_r  r_density    max_v   flags               color
    {"border",  0.,         0,          0.,         1000.,        0.,    PIXEL_FLAG_IMMOVABLE, 0x111111FF, 0x222222FF, 0x333333FF },
    {"air"  ,   0.,         0,          0.,         0.00124,      0.,    PIXEL_FLAG_SKIP, 0xFFFFFF00, 0x88888800, 0x00000000},
    {"rock" ,   1.,         0,          0.,         2.5,         20.,    PIXEL_FLAG_IMMOVABLE, 0X929292FF, 0x5B5B5Bff, 0x4E4E4Eff }, 
    {"water",   0.5,        3,          0.,         1.,          10.,    PIXEL_FLAG_PHYSICAL|PIXEL_FLAG_DIAGONAL_CHECK|PIXEL_FLAG_FLUID,0x0c71c301,0x0c71c301,0x0c71c301},
    {"sand" ,   0.17,       0,          0.1,        1.4,         15.,    PIXEL_FLAG_PHYSICAL | PIXEL_FLAG_DIAGONAL_CHECK | PIXEL_FLAG_VMC, 0xFFFF00FF, 0xb2c906ff, 0xe9fc5aff },
    {"dirt" ,   0.4,        0,          0.65,       1.22,        10.,    PIXEL_FLAG_PHYSICAL | PIXEL_FLAG_DIAGONAL_CHECK | PIXEL_FLAG_VMC, 0x602f12FF, 0x874620ff, 0x4f260fff },
    {"coal" ,   0.4,        0,          0.68,       1.32,        10.,    PIXEL_FLAG_PHYSICAL | PIXEL_FLAG_DIAGONAL_CHECK | PIXEL_FLAG_VMC, 0x353535FF, 0x222326ff, 0x414141ff },
    {"sawdust", 0.4,        0,          0.7,        0.21,         4.,    PIXEL_FLAG_PHYSICAL | PIXEL_FLAG_DIAGONAL_CHECK | PIXEL_FLAG_VMC, 0xF1BE69FF, 0xdaa96aff, 0xaf8136ff },
    {"smoke",   0.,         3,          0.,         0.0006,       2.,    PIXEL_FLAG_PHYSICAL | PIXEL_FLAG_DIAGONAL_CHECK|PIXEL_FLAG_FLUID, 0x0eeeeeeFF,0xccccccFF,0x969292FF},
    {"funky",   1.,         3,          0.,         0.5,       1000.,    PIXEL_FLAG_PHYSICAL|PIXEL_FLAG_DIAGONAL_CHECK|PIXEL_FLAG_FUNKY|PIXEL_FLAG_VMC,0xff00f9FF,0xff20d9FF,0xdf00f9FF },
    {"mercury", 0.1,        1,          0.,         13.534,       5.,    PIXEL_FLAG_PHYSICAL|PIXEL_FLAG_DIAGONAL_CHECK|PIXEL_FLAG_FLUID|PIXEL_FLAG_VMC,0xf7f8f9ff,0x999988ff,0xFFFFFFff}
};
//sawdust #9A9174 #9C9172 #AC8059
//good tree bark color : 63331B
// 0xb5a849ff
cell cell_from_type(int type) {
    float v = rng();
    int color = ( (v < .33) ? pixelTypes[type].color1 : ( (v > .67) ? pixelTypes[type].color3 : pixelTypes[type].color2 ) );
    return {type, vec2f(0,0), vec2f(0,0), true, color, false, false};
}

bool hasFlags(int type, int flags) {
    if (flags == PIXEL_FLAG_SKIP) return (pixelTypes[type].flags == PIXEL_FLAG_SKIP);
    else return ( (pixelTypes[type].flags & flags) == flags);
}

bool hasFlags(cell c, int flags) {
    return hasFlags(c.type, flags);
}

void print( cell c) {
    std::cout << " | " << pixelTypes[c.type].name << " - vel :";
    c.vel.print();
    std::cout << " - precision_bucket : ";
    c.precision_bucket.print();
    std::cout << " - isActive : " << (c.isActive ? "true" : "false")  << " | " ;
    
}

bool is_border(cell c) {
    return (c.type == PIXEL_TYPE_BORDER);
}
bool is_empty(cell c) {
    return (c.type == PIXEL_TYPE_AIR);
}

class cell_grid {
public:
    uint w = 64;
    uint h = 64;
    bool clock = true;
    cell* cells = NULL; //cells[y][x]
    
    void free_cells() {
        if (cells != NULL)    free(cells);
    }
    void allocate() {
        free_cells();
        cells = (cell*) calloc(w*h, sizeof(cell)); //fill everything to 0
        for (int i = 0; i < w*h ; i++) {
            cells[i].type = PIXEL_TYPE_AIR;
            cells[i].isActive = true;
        }
    }
    void reset() {
        allocate();
    }
    ~ cell_grid () {
        free_cells();
    }
    cell_grid() {
        allocate();
    }
    cell_grid(uint W, uint H) {
        w = W;
        h = H;
        allocate();
    }
    bool inBound (uint y, uint x) {
        return !(y < 0 || y >= h || x <0 || x>= w);
    }
    cell get (uint y, uint x) {
        if (! inBound(y,x) ) {
            return cell_from_type(PIXEL_TYPE_BORDER);
        } else {
            return cells[y*w + x];
        }
    }
    bool interact (uint y, uint x) {
        if ( inBound(y, x) ) {
            if ((!cells[y*w + x].isActive) && (rng() > pixelTypes[cells[y*w + x].type].inertialResistance ) ) { //tries to be waken up
                cells[y*w + x].isActive = true;
            }
            return cells[y*w + x].isActive;
        } 
        return false;
    }
    cell set (uint y, uint x, cell c) { //set & return what was in its place before
        if (! inBound(y,x)) {
            return cell_from_type(PIXEL_TYPE_BORDER);
        } else {
            cell t = cells[y*w + x];
            cells[y*w + x] = c;
            return t;
        }
    }
    float relative_density (uint y, uint x) {
        return pixelTypes[get(y,x).type].relative_density;
    }
    float max_speed (uint y, uint x) {
        return pixelTypes[get(y,x).type].max_speed;
    }
    float friction (uint y, uint x) {
        return pixelTypes[get(y,x).type].friction;
    }
    float inertialResistance (uint y, uint x) {
        return pixelTypes[get(y,x).type].inertialResistance;
    }
    float apparent_density (uint y, uint x) {
        return relative_density(y, x) / pixelTypes[PIXEL_TYPE_AIR].relative_density;
    }
    int rise (uint y, uint x) {
        return (apparent_density(y, x) > 1) ? 1 : -1;
    }
    int matrix_traversal(uint y, uint x, int dy, int dx) { //return the amount travelled in the "longest" direction -> nope; return  if no movements, 1 if full, -1 if not full
        if (dy == 0 && dx == 0) return 0;
        int ay = std::abs(dy);
        int ax = std::abs(dx);
        int x_sign = (dx >= 0) ? 1 : -1;
        int y_sign = (dy >= 0) ? 1 : -1;
        bool x_bigger = (ax >= ay);
        float slope = (x_bigger ? dy/dx : dx/dy) ;
        int length = (x_bigger ? ax : ay);
        int ds = 0;
        
        //int dsp = 0;
        //std::cout << " matrix traversiiing : length = " << length << ", dx = " << dx << ", dy = " << "\n";
        for (int i = 1; i <= length; i++) {
            ds = std::round(slope * i); //delta smol
            //std::cout << i << " out of " << length << ", with ds = " << ds << " ! \n";
            if (x_bigger) {
                if ( ! is_empty(get(y+ y_sign*ds, x+ x_sign*i)) ) {
                    if ( !move(y, x, y_sign*ds, x_sign*i) ) { //if i can't move to that pixel
                        ds = std::round(slope * (i-1));
                        return -move( y, x, y_sign*ds, x_sign*(i-1) ); //i'll move to the one before that
                    } else {
                        return -1;
                    }
                }
            } else {
                if ( ! is_empty(get(y+ y_sign*i, x+ x_sign*ds)) ) {
                    if ( !move(y, x, y_sign*i, x_sign*ds) ) {//if i can't move to that pixel
                        ds = std::round(slope * (i-1));
                        return -move( y, x, y_sign*(i-1), x_sign*ds ); //i'll move to the one before that
                    } else {
                        return -1;
                    }
                }
            }
        }
        return move(y, x, dy, dx);
    }
    bool move(uint y, uint x, int dy, int dx) {
        //here, |dx|, |dy| <= 1
        cell target = get(y+dy, x+dx);
        
        if ( is_border(target) || hasFlags(target, PIXEL_FLAG_IMMOVABLE) ) { //blocked, most likely out of bounds
            cells[y*w + x].movedYLastFrame = false;
            return false;
        } else if ( is_empty(target) ) { //"classic" fall
            set(y+dy, x+dx, get(y,x));
            set(y,   x, target  );
            cells[y*w + x].movedYLastFrame = (dy != 0);
            return true;
        } else if (hasFlags(get(y,x), PIXEL_FLAG_FLUID) || hasFlags(target, PIXEL_FLAG_FLUID) ) { 
            //density check -> sinking ; only used for "static like" movements ; not between two solids
            //float v = dy * ( relative_density(y, x) -  pixelTypes[target.type].relative_density );
            float v = ( relative_density(y, x) /  pixelTypes[target.type].relative_density );
            v = ((dy == 0 ) ? 1 : dy )*std::log(v);
            //v /= 1 + std::abs(dx); //need much more delta(density) to push blocks nearby
            if ( v > rng() ) { //test passed
                set(y+dy, x+dx, get(y,x));
                set(y,   x, target  ); //is moved once to make sure we dont duplicate/delete
                
                int dx_ = (rng(2)==1) ? 1 : -1; //try to go on the side afterward, if possible
                move(y, x, dy, dx+dx_); //recursive might not be a good idea, but fine for now
                cells[y*w + x].movedYLastFrame = (dy != 0);
                return true;
            }
        }
        cells[y*w + x].movedYLastFrame = false;
        return false;
    }
    bool sand(uint y, uint x) {
        if ( move(y,x,1,0) ) {
            return true;
        } else {
            int dx = (rng(2)==1) ? 1 : -1;
            return  move(y,x,1,dx);//slight flaw : does not check the other side
        }
        return false;
    }
    bool liquid(uint y, uint x) {
        int dy = (apparent_density(y, x) >= 1 ) ? 1 : -1 ;
        if ( move(y,x,dy,0) ) {
            return true;
        } else {
            int dx = (rng()>.5) ? 1 : -1;
            if ( move(y,x,dy,dx) || move(y,x,dy,-dx) ) { //abusing lazy operator
                return true;
            } else {
                int v = rng(pixelTypes[get(y,x).type].dispersion_rate) +1;
                return (matrix_traversal(y, x, 0, dx*v ) != 0);
            }
        }
        return false;
    }
    bool funky(uint y, uint x) {
        if ( move(y,x,1,0) ) {
            return true;
        } else {
            int dx = (rng()>.5) ? 1 : -1;
            return ( (matrix_traversal(y, x, -4, 1*dx) != 0) );
        }
    }
    bool physical(uint y, uint x) {
        if( is_border(get(y, x)) ) return false;
        vec2f acc = vec2f(0, std::log(apparent_density(y, x)) ) ; //TODO : add more parameter for accuracy/homogeneity
        vec2f v = cells[y*w + x].vel + acc * .018;
        //v.print();
        v.clamp(max_speed(y,x));
        cells[y*w + x].vel = v;
        int dx = std::round(v.x);
        int dy = std::round(v.y);
        int res = matrix_traversal(y, x, dy, dx );
        
        //TODO : fix bucket handling
        bool bucket_move = false;
        /*
        if ( std::abs(dx) == 0 ) cells[y*w + x].precision_bucket.x += v.x;
        if ( std::abs(dy) == 0 ) cells[y*w + x].precision_bucket.y += v.y;
        if ( std::abs(cells[y*w + x].precision_bucket.x) >= 1 ) { //if precision_bucket is full
            bucket_move = bucket_move || move(y, x, 0, std::round(cells[y*w + x].precision_bucket.x) ); //try to move
            cells[y*w + x].precision_bucket.x = 0; //and empty that bastard
        }
        if ( std::abs(cells[y*w + x].precision_bucket.y) >= 1 ) { //if precision_bucket is full
            bucket_move = bucket_move || move(y, x, std::round(cells[y*w + x].precision_bucket.y), 0 ); //try to move
            cells[y*w + x].precision_bucket.y = 0; //and empty that bastard
        }
        */
        //std::cout << " x " << x << " ; y " << y << " ; dx " << dx << " ; dy " << dy << " ; res " << res << " \n";
        //if the movement encountered an obstacle TODO : once matrix_traversal is better handled, implement for other movements than horizontal (friction)
            
        // -----------------------------------------------------------------------------------------------------------------------
        
        //physicaly stopped -> stopped moving && was moving significantly && moved some bit
        
        if (!is_empty(get(y+rise(y,x), x)) ) {//if is above ground
            
            //apply friction
            cells[y*w + x].vel.x *= (1 - friction(y,x) ); //TODO : multiply by neighbour's friction
            
            //cells[y*w + x].vel.print(); std::cout << " x " << x << " ; y " << y << " ; dx " << dx << " ; dy " << dy << " ; res " << res << " \n";
            
            if ((cells[y*w + x].vel.squareDistance() >= 1) || cells[y*w + h].movedYLastFrame ) { //( (dx*dx + dy*dy >= 1) && (res != 0) ) { //if the object was physicaly stopped (not just too slow) 
                //cells[y*w + x].vel.print(); std::cout << "\n";
                if (hasFlags(cells[y*w + x], PIXEL_FLAG_VMC) ) { //TODO : make VMC & friction work for other directions than down
                    
                    float dx;
                    if (cells[y*w + x].vel.x == 0) {
                        dx = (rng(2)==1) ? 1 : -1;
                    } else {
                        dx = (cells[y*w + x].vel.x > 0) ? 1 : -1;
                    }
                    dx *= cells[y*w + x].vel.y / 3; //TODO : make parameter more "global" -> "bouncy" factor ?
                    cells[y*w + x].vel.x += dx;
                    //if (cells[y*w + x].vel.x != 0) std::cout << "new x speed : " << cells[y*w + x].vel.x << "dx : " << dx << " \n";
                } else {
                    cells[y*w + x].vel = vec2f(0, 0); 
                }
            }
            cells[y*w + x].vel.y = 0;
        }
        // -----------------------------------------------------------------------------------------------------------------------
        
        return (res != 0) || bucket_move;
        
    }
    bool step(uint y, uint x) {
        cell c = get(y, x);
        if ( hasFlags(c, PIXEL_FLAG_IMMOVABLE) || hasFlags(c, PIXEL_FLAG_SKIP) ) return false;
        //std::cout << c.clock << " ; " << clock << " | ";
        if ( c.clock != clock ) { //if not moved yet
            cells[y*w + x].clock = clock; //update clock
            
            if ( ! c.isActive ) { //if this cell is currently not moving
                if ( is_empty(get(y+rise(y,x), x)) || 
                    (hasFlags(c, PIXEL_FLAG_FLUID) && 
                        (is_empty(get(y+1, x+1)) || is_empty(get(y+1, x-1)) || is_empty(get(y, x+1)) || is_empty(get(y, x-1)) ))  ) { 
                        //and the cell below is empty (or the cells around, if fluid)
                    //std::cout << y << " ; " << x << "\n";
                    cells[y*w + x].isActive = true; //then wake up
                } else {
                    return false; //going back to bed
                }
            }
            //interact with neighbours
            interact(y+1, x);
            interact(y-1, x);
            interact(y, x+1);
            interact(y, x-1);
            
            
            if ( hasFlags(c, PIXEL_FLAG_PHYSICAL) ) {
                if (physical(y, x)) {
                    return true;
                } 
                if ( hasFlags(c, PIXEL_FLAG_FLUID) ) {
                    return liquid(y, x);
                }
                if ( hasFlags(c, PIXEL_FLAG_FUNKY) ) {
                    return funky(y, x);
                } else if ( hasFlags(c, PIXEL_FLAG_DIAGONAL_CHECK) ) {
                    return sand(y, x);
                }
            }
        }
        return false;
    }
    void step() {
        for(uint y = 0; y < h; y++) {
            for(uint x = 0; x < w; x++) {
                if (step(y, x)) { //if moved
                    cells[y*w + x].isActive = true;
                } else {
                    cells[y*w + x].isActive = false;                    
                }
            }
        }
        clock = 1 - clock;
    }
    cell * operator[] (uint y) {
        return cells + y*w;
    }
    void draw_circle(int type, uint x, uint y, float r) {
        for (int i = (int)(-r/2); i < r/2; i++) {
            for (int j = (int)(-r/2); j < r/2; j++) {
                if (i*i + j*j <= r*r/4) { //if in circle
                    if (get(y+i, x+j).type != type ) set(y+i, x+j, cell_from_type(type));
                }
            }
        }
    }
    void draw_square(int type, uint x, uint y, int r) {
        for (int i = -r/2; i < r/2; i++) {
            for (int j = -r/2; j < r/2; j++) {
                if (get(y+i, x+j).type != type ) set(y+i, x+j, cell_from_type(type));
            }
        }
    }
    
    void symetrize() {
        for(uint y = 0; y < h; y++) {
            for(uint x = 0; x < w/2; x++) {
                cell c = get(y, w - x);
                if (! is_border(c) ) set(y, x, c);
            }
        }
    }
    
    //void draw(int o_x, int o_y, float scale) {};
};





#endif
