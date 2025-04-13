#pragma once
#include "OpenGLwin.h"
#include "HashTable.h"
#include "Heap.h"
#include "BitArray.h"

struct Quadric {
    f64 a2, b2, c2, d2;
    f64 ab, ac, ad;
    f64 bc, bd, cd;
    Quadric() { memset(this, 0, sizeof(f64) * 10); }
    Quadric(dvec3 p0, dvec3 p1, dvec3 p2) {
        dvec3 n = Normalize(Cross(p1 - p0, p2 - p0));
        auto [a, b, c] = n;
        f64 d = -Dot(n, p0);
        a2 = a * a, b2 = b * b, c2 = c * c, d2 = d * d;
        ab = a * b, ac = a * c, ad = a * d;
        bc = b * c, bd = b * d, cd = c * d;
    }
    void Add(Quadric b) {
        f64* t1 = (f64*)this;
        f64* t2 = (f64*)&b;
        for (u32 i = 0; i < 10; i++) t1[i] += t2[i];
    }
    bool Get(vec3& p) {
        dmat4 m, inv;
        m.SetCol(0, { a2,ab,ac,0 });
        m.SetCol(1, { ab,b2,bc,0 });
        m.SetCol(2, { ac,bc,c2,0 });
        m.SetCol(3, { ad,bd,cd,1 });
        if (!Invert(m, inv)) return false;
        dvec4 v = inv.col[3];
        p = { (f32)v.x,(f32)v.y,(f32)v.z };
        return true;
    }
    f32 Evaluate(vec3 p) {
        f32 res = a2 * p.x * p.x + 2 * ab * p.x * p.y + 2 * ac * p.x * p.z + 2 * ad * p.x
            + b2 * p.y * p.y + 2 * bc * p.y * p.z + 2 * bd * p.y
            + c2 * p.z * p.z + 2 * cd * p.z + d2;
        return res <= 0.f ? 0.f : res;
    }
};


class MeshSimplifier {
    MeshSimplifier* impl;
public:
    MeshSimplifier() { impl = nullptr; }
    MeshSimplifier(Vertex* _verts, u32 _numVert, u32* _indexes, u32 _numIndex);
    ~MeshSimplifier();

    void LockPosition(vec3 p);
    void Simplify(u32 targetNumTri);
    u32 RemainingNumVert();
    u32 RemainingNumTri();
    f32 MaxError();
};

class MeshSimplifierImpl final : public MeshSimplifier {
public:
    u32 numVert;
    u32 numIndex;
    u32 numTri;

    Vertex* verts;
    u32* indexes;

    HashTable vertexHashTable;
    HashTable cornerHashTable;
    vector<u32> vertRefs;
    vector<u8> flags;
    BitArray triRemoved;

    enum flag {
        AdjMask = 1,
        LockMask = 2
    };

    vector<pair<Vertex, Vertex>> edges;
    HashTable fromHashTable;
    HashTable toHashTable;
    Heap heap;

    vector<u32> moveVert;
    vector<u32> moveCorner;
    vector<u32> moveEdge;
    vector<u32> reEvaluateEdges;

    vector<Quadric> triQuadrics;

    f32 maxError;
    u32 remainingNumVert;
    u32 remainingNumTri;

    MeshSimplifierImpl(Vertex* _verts, u32 _numVert, u32* _indexes, u32 _numIndex);
    ~MeshSimplifierImpl() {}

    u32 Hash(const vec3& v) {
        union { f32 f; u32 u; } x, y, z;
        x.f = (v.x == 0.f ? 0 : v.x);
        y.f = (v.y == 0.f ? 0 : v.y);
        z.f = (v.z == 0.f ? 0 : v.z);
        return MurMurMix(MurMurAdd(MurMurAdd(x.u, y.u), z.u));
    }
    void SetVertIdx(u32 corner, u32 idx);
    void RemoveIfVertDuplicate(u32 corner);
    bool IsTriDuplicate(u32 triIdx);
    void FixupTri(u32 triIdx);
    bool AddEdgeHashTable(vec3& p0, vec3& p1, u32 idx);

    void GatherAdjTris(vec3 p, vector<u32>& tris, bool& lock);
    f32 Evaluate(Vertex p0, Vertex p1, bool merge);
    void LockPosition(vec3 p);
    // bool is_position_locked(vec3 p);
    void Simplify(u32 targetNumTri);
    void Compact();

    void BeginMerge(vec3 p);
    void EndMerge();
};