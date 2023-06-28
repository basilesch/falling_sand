#include <vector>
#include <math.h>
#include <iostream>

#ifndef VEC2F_HPP
#define VEC2F_HPP

class vec2f {
    public :
        ~ vec2f() {}
        double x = 0;
        double y = 0;
        vec2f () {}
        vec2f (double _x, double _y) {
            x = _x;
            y = _y;
        }
        vec2f operator*(double s) {
            return vec2f(x*s,y*s);
        }
        vec2f operator+(vec2f v2) {
            return vec2f(x+v2.x,y+v2.y);
        }
        void operator+=(vec2f v2) {
            x += v2.x;
            y += v2.y;
        }
        void operator-=(vec2f v2) {
            x -= v2.x;
            y -= v2.y;
        }
        void operator*=(double s) {
            x *= s;
            y *= s;
        }
        vec2f operator-(vec2f v2) {
            return vec2f(x-v2.x,y-v2.y);
        }
        double squareDistance() {
            return x*x+y*y;
        }
        double squareDistance(vec2f v2) {
            double X = x - v2.x;
            double Y = y - v2.y;
            return X*X+Y*Y;
        }
        double distance(vec2f v2) {
            double X = x - v2.x;
            double Y = y - v2.y;
            return sqrt(X*X+Y*Y);
        }
        double size() {
            return sqrt(squareDistance());
        }
        void scale(double a) {
            double s = size();
            if (s != 0) *this *= a/s;
        }
        void normalize() {
            scale(1);
        }
        void clamp(double a) {
            if (squareDistance() > a*a) scale(a);
        }
        double scalaire(vec2f v2) {
            return x*v2.x + y*v2.y;
        }
        void print() {
            printf(" (%f, %f) ", x, y);
        }
};

#endif
