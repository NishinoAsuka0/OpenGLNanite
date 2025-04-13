#pragma once
#include"OpenGLwin.h"

struct Bounds {
    vec3 pmin, pmax;
    Bounds() { pmin = { 1e9, 1e9, 1e9 }, pmax = { -1e9, -1e9, -1e9 }; }
    Bounds(vec3 p) { pmin = p, pmax = p; }
    Bounds operator+(Bounds b);
    Bounds operator+(vec3 b);
};

struct Sphere {
    vec3 center;
    f32 radius;

    Sphere operator+(Sphere b);
    static Sphere FromPoints(Vertex* verts, u32 size);
    static Sphere FromSpheres(Sphere* spheres, u32 size);
};
