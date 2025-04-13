#include "Bounds.h"

inline f32 Sqr(f32 x) { return x * x; }

Bounds Bounds::operator+(Bounds b)
{
    Bounds bounds;
    bounds.pmin = vec3(
        min(pmin.x, b.pmin.x),
        min(pmin.y, b.pmin.y),
        min(pmin.z, b.pmin.z)
   );

    bounds.pmax = vec3(
        max(pmax.x, b.pmax.x),
        max(pmax.y, b.pmax.y),
        max(pmax.z, b.pmax.z)
    );
    return bounds;
}

Bounds Bounds::operator+(vec3 b)
{
    Bounds bounds;
    bounds.pmin = vec3(
        min(pmin.x, b.x),
        min(pmin.y, b.y),
        min(pmin.z, b.z)
    );

    bounds.pmax = vec3(
        max(pmax.x, b.x),
        max(pmax.y, b.y),
        max(pmax.z, b.z)
    );
    return bounds;
}
Sphere Sphere::operator+(Sphere b)
{
    vec3 t = b.center - center;
    f32 tLen2 = Length2(t);
    if (Sqr(radius - b.radius) >= tLen2) {
        return radius < b.radius ? b : *this;
    }
    Sphere sphere;
    f32 tLen = sqrt(tLen2);
    sphere.radius = (tLen + radius + b.radius) * 0.5f;
    sphere.center = center + t * ((sphere.radius - radius) / tLen);
    return sphere;
}
Sphere Sphere::FromPoints(Vertex* verts, u32 size)
{
    //找出三个轴上最远/近的点
    u32 minIdx[3] = {};
    u32 maxIdx[3] = {};
    for (u32 i = 0; i < size; i++) {
        for (int k = 0; k < 3; k++) {
            if (verts[i].pos[k] < verts[minIdx[k]].pos[k]) minIdx[k] = i;
            if (verts[i].pos[k] > verts[maxIdx[k]].pos[k]) maxIdx[k] = i;
        }
    }

    //计算最远的的距离在哪个轴上
    f32 maxLen = 0;
    u32 maxAxis = 0;
    for (u32 k = 0; k < 3; k++) {
        vec3 pMin = verts[minIdx[k]].pos;
        vec3 pMax = verts[maxIdx[k]].pos;
        f32 tLen = Length2(pMax - pMin);
        if (tLen > maxLen) {
            maxLen = tLen;
            maxAxis = k;
        }
    }
    vec3 pMin = verts[minIdx[maxAxis]].pos;
    vec3 pMax = verts[maxIdx[maxAxis]].pos;

    //创建最小包围球
    Sphere sphere;
    sphere.center = (pMin + pMax) * 0.5f;
    sphere.radius = f32(0.5 * sqrt(maxLen));
    maxLen = sphere.radius * sphere.radius;

    for (u32 i = 0; i < size; ++i) {
        f32 len = Length2(verts[i].pos - sphere.center);
        if (len > maxLen) {
            len = sqrt(len);
            f32 t = 0.5 - 0.5 * (sphere.radius / len);
            sphere.center = sphere.center + (verts[i].pos - sphere.center) * t;
            sphere.radius = (sphere.radius + len) * 0.5;
            maxLen = sphere.radius * sphere.radius;
        }
    }

    /*for (u32 i = 0; i < size; ++i) {
        f32 len = Length(pos[i] - sphere.center);
        assert(len <= sphere.radius);
    }*/
    return sphere;
}

Sphere Sphere::FromSpheres(Sphere* spheres, u32 size)
{
    u32 minIdx[3] = {};
    u32 maxIdx[3] = {};
    for (u32 i = 0; i < size; ++i) {
        for (u32 k = 0; k < 3; ++k) {
            if (spheres[i].center[k] - spheres[i].radius 
                < spheres[minIdx[k]].center[k] - spheres[minIdx[k]].radius
                )
                minIdx[k] = i;
            if (spheres[i].center[k] + spheres[i].radius 
                < spheres[maxIdx[k]].center[k] + spheres[maxIdx[k]].radius
                )
                maxIdx[k] = i;
        }
    }

    f32 maxLen = 0;
    u32 maxAxis = 0;
    for (u32 k = 0; k < 3; k++) {
        Sphere spmin = spheres[minIdx[k]];
        Sphere spmax = spheres[maxIdx[k]];
        f32 tlen = Length(spmax.center - spmin.center) + spmax.radius + spmin.radius;
        if (tlen > maxLen) maxLen = tlen, maxAxis = k;
    }
    Sphere sphere = spheres[minIdx[maxAxis]];
    sphere = sphere + spheres[maxIdx[maxAxis]];

    for (u32 i = 0; i < size; ++i) {
        sphere = sphere + spheres[i];
    }

    for (u32 i = 0; i < size; ++i) {
        f32 t1 = Sqr(sphere.radius - spheres[i].radius);
        f32 t2 = Length2(sphere.center - spheres[i].center);
        //assert(t1 + 1e-6 >= t2);
    }
    return sphere;
}
