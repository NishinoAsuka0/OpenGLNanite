#include "MeshSimplify.h"


MeshSimplifierImpl::MeshSimplifierImpl(vec3* _verts, u32 _numVert, u32* _indexes, u32 _numIndex)
    :numVert(_numVert), numIndex(_numIndex), numTri(numIndex / 3)
    , verts(_verts), indexes(_indexes)
    , vertexHashTable(numVert), vertRefs(numVert), cornerHashTable(numIndex)
    , triRemoved(numTri), flags(numIndex)
{
    remainingNumVert = numVert, remainingNumTri = numTri;
    for (u32 i = 0; i < numVert; i++) {
        vertexHashTable.Add(Hash(verts[i]), i);
    }

    u32 expNumEdge = min(min(numIndex, 3 * numVert - 6), numTri + numVert);
    edges.reserve(expNumEdge);
    fromHashTable.Resize(expNumEdge);
    toHashTable.Resize(expNumEdge);

    for (u32 corner = 0; corner < numIndex; corner++) {
        u32 v_idx = indexes[corner];
        vertRefs[v_idx]++;
        const vec3& p = verts[v_idx];
        cornerHashTable.Add(Hash(p), corner);

        vec3 p0 = p;
        vec3 p1 = verts[indexes[Cycle3(corner)]];
        if (AddEdgeHashTable(p0, p1, edges.size())) {
            edges.push_back({ p0,p1 });
        }
    }
}

void MeshSimplifierImpl::LockPosition(vec3 p) {
    for (u32 i : cornerHashTable[Hash(p)]) {
        if (verts[indexes[i]] == p) {
            flags[i] |= LockMask;
        }
    }
}

bool MeshSimplifierImpl::AddEdgeHashTable(vec3& p0, vec3& p1, u32 idx) {
    u32 h0 = Hash(p0), h1 = Hash(p1);
    if (h0 > h1) swap(h0, h1), swap(p0, p1);
    for (u32 i : fromHashTable[h0]) {
        auto& e = edges[i];
        if (e.first == p0 && e.second == p1) return false;
    }
    fromHashTable.Add(h0, idx);
    toHashTable.Add(h1, idx);
    return true;
}

void MeshSimplifierImpl::SetVertIdx(u32 corner, u32 idx) {
    u32& vertexIdx = indexes[corner];
    assert(vertexIdx != ~0u);
    assert(vertRefs[vertexIdx] > 0);

    if (vertexIdx == idx) return;
    if (--vertRefs[vertexIdx] == 0) {
        vertexHashTable.Remove(Hash(verts[vertexIdx]), vertexIdx);
        remainingNumVert--;
    }
    vertexIdx = idx;
    if (vertexIdx != ~0u) vertRefs[vertexIdx]++;
}

//将corner赋值为第一个遇到的相同点
void MeshSimplifierImpl::RemoveIfVertDuplicate(u32 corner) {
    u32 vertexIdx = indexes[corner];
    vec3& v = verts[vertexIdx];
    for (u32 i : vertexHashTable[Hash(v)]) {
        if (i == vertexIdx) break;
        if (v == verts[i]) {
            SetVertIdx(corner, i);
            break;
        }
    }
}

bool MeshSimplifierImpl::IsTriDuplicate(u32 triIdx) {
    u32 i0 = indexes[triIdx * 3 + 0], i1 = indexes[triIdx * 3 + 1], i2 = indexes[triIdx * 3 + 2];
    for (u32 i : cornerHashTable[Hash(verts[i0])]) {
        if (i != triIdx * 3) {
            if (i0 == indexes[i] && i1 == indexes[Cycle3(i)] && i2 == indexes[(Cycle3(i, 2))])
                return true;
        }
    }
    return false;
}

void MeshSimplifierImpl::FixupTri(u32 triIdx) {
    assert(!triRemoved[triIdx]);

    const vec3& p0 = verts[indexes[triIdx * 3 + 0]];
    const vec3& p1 = verts[indexes[triIdx * 3 + 1]];
    const vec3& p2 = verts[indexes[triIdx * 3 + 2]];

    bool isRemoved = false;
    if (!isRemoved) {
        isRemoved = (p0 == p1) || (p1 == p2) || (p2 == p0);
    }
    if (!isRemoved) {
        for (u32 k = 0; k < 3; k++) RemoveIfVertDuplicate(triIdx * 3 + k);
        isRemoved = IsTriDuplicate(triIdx);
    }
    if (isRemoved) {
        triRemoved.SetTrue(triIdx);
        remainingNumTri--;
        for (u32 k = 0; k < 3; k++) {
            u32 corner = triIdx * 3 + k;
            u32 v_idx = indexes[corner];
            cornerHashTable.Remove(Hash(verts[v_idx]), corner);
            SetVertIdx(corner, ~0u);
        }
    }
    else triQuadrics[triIdx] = Quadric(p0, p1, p2);
}

void MeshSimplifierImpl::GatherAdjTris(vec3 p, vector<u32>& tris, bool& lock) {
    for (u32 i : cornerHashTable[Hash(p)]) {
        if (verts[indexes[i]] == p) {
            u32 triIdx = i / 3;
            if ((flags[triIdx * 3] & AdjMask) == 0) {
                flags[triIdx * 3] |= AdjMask;
                tris.push_back(triIdx);
            }
            if (flags[i] & LockMask) {
                lock = true;
            }
        }
    }
}

float MeshSimplifierImpl::Evaluate(vec3 p0, vec3 p1, bool merge) {
    if (p0 == p1) return 0.f;

    f32 error = 0;

    vector<u32> adjTris;
    bool lock0 = false, lock1 = false;
    GatherAdjTris(p0, adjTris, lock0);
    GatherAdjTris(p1, adjTris, lock1);
    if (adjTris.size() == 0) return 0.f;
    if (adjTris.size() > 24) {
        error += 0.5 * (adjTris.size() - 24);
    }

    Quadric q;
    for (u32 i : adjTris) {
        q.Add(triQuadrics[i]);
    }
    vec3 p = (p0 + p1) * 0.5f;

    auto is_valid_pos = [&](vec3 p)->bool {
        if (Length(p - p0) + Length(p - p1) > 2 * Length(p0 - p1))
            return false;
        return true;
        };

    if (lock0 && lock1) error += 1e8;
    if (lock0 && !lock1) p = p0;
    else if (!lock0 && lock1) p = p1;
    else if (!q.Get(p)) p = (p0 + p1) * 0.5f;
    if (!is_valid_pos(p)) {
        p = (p0 + p1) * 0.5f;
    }
    error += q.Evaluate(p);

    if (merge) {
        BeginMerge(p0), BeginMerge(p1);
        for (u32 i : adjTris) {
            for (u32 k = 0; k < 3; k++) {
                u32 corner = i * 3 + k;
                vec3& pos = verts[indexes[corner]];
                if (pos == p0 || pos == p1) {
                    pos = p;
                    if (lock0 || lock1) flags[corner] |= LockMask;
                }
            }
        }
        for (u32 i : moveEdge) {
            auto& edge = edges[i];
            if (edge.first == p0 || edge.first == p1) edge.first = p;
            if (edge.second == p0 || edge.second == p1) edge.second = p;
        }
        EndMerge();

        vector<u32> adjVerts;
        for (u32 i : adjTris) {
            for (u32 k = 0; k < 3; k++) {
                adjVerts.push_back(indexes[i * 3 + k]);
            }
        }
        sort(adjVerts.begin(), adjVerts.end());
        adjVerts.erase(unique(adjVerts.begin(), adjVerts.end()), adjVerts.end());

        for (u32 vertexIdx : adjVerts) {
            u32 hashNum = Hash(verts[vertexIdx]);
            for (u32 i : fromHashTable[hashNum]) {
                if (edges[i].first == verts[vertexIdx]) {
                    if (heap.IsPresent(i)) {
                        heap.Remove(i);
                        reEvaluateEdges.push_back(i);
                    }
                }
            }
            for (u32 i : toHashTable[hashNum]) {
                if (edges[i].second == verts[vertexIdx]) {
                    if (heap.IsPresent(i)) {
                        heap.Remove(i);
                        reEvaluateEdges.push_back(i);
                    }
                }
            }
        }
        for (u32 i : adjTris) {
            FixupTri(i);
        }
    }
    for (u32 i : adjTris) {
        flags[i * 3] &= (~AdjMask);
    }
    return error;
}

void MeshSimplifierImpl::BeginMerge(vec3 p) {
    u32 h = Hash(p);
    for (u32 i : vertexHashTable[h]) {
        if (verts[i] == p) {
            vertexHashTable.Remove(h, i);
            moveVert.push_back(i);
        }
    }
    for (u32 i : cornerHashTable[h]) {
        if (verts[indexes[i]] == p) {
            cornerHashTable.Remove(h, i);
            moveCorner.push_back(i);
        }
    }
    for (u32 i : fromHashTable[h]) {
        if (edges[i].first == p) {
            fromHashTable.Remove(Hash(edges[i].first), i);
            toHashTable.Remove(Hash(edges[i].second), i);
            moveEdge.push_back(i);
        }
    }
    for (u32 i : toHashTable[h]) {
        if (edges[i].second == p) {
            fromHashTable.Remove(Hash(edges[i].first), i);
            toHashTable.Remove(Hash(edges[i].second), i);
            moveEdge.push_back(i);
        }
    }
}

void MeshSimplifierImpl::EndMerge() {
    for (u32 i : moveVert) {
        vertexHashTable.Add(Hash(verts[i]), i);
    }
    for (u32 i : moveCorner) {
        cornerHashTable.Add(Hash(verts[indexes[i]]), i);
    }
    for (u32 i : moveEdge) {
        auto& e = edges[i];
        if (e.first == e.second || !AddEdgeHashTable(e.first, e.second, i)) {
            heap.Remove(i);
        }
    }
    moveVert.clear();
    moveCorner.clear();
    moveEdge.clear();
}

void MeshSimplifierImpl::Simplify(u32 targetNumTri) {
    triQuadrics.resize(numTri);
    for (u32 i = 0; i < numTri; i++) FixupTri(i);
    if (remainingNumTri <= targetNumTri) {
        Compact();
        return;
    }
    heap.Resize(edges.size());
    u32 i = 0;
    for (auto& edge : edges) {
        f32 error = Evaluate(edge.first, edge.second, false);
        heap.Add(error, i);
        i++;
    }

    maxError = 0;
    while (!heap.Empty()) {
        u32 edgeIdx = heap.Top();
        if (heap.GetKey(edgeIdx) >= 1e6) break;

        heap.Pop();

        auto& edge = edges[edgeIdx];
        fromHashTable.Remove(Hash(edge.first), edgeIdx);
        toHashTable.Remove(Hash(edge.second), edgeIdx);

        f32 error = Evaluate(edge.first, edge.second, true);
        if (error > maxError) maxError = error;

        if (remainingNumTri <= targetNumTri) break;

        for (u32 i : reEvaluateEdges) {
            auto& e = edges[i];
            f32 error = Evaluate(e.first, e.second, false);
            heap.Add(error, i);
        }
        reEvaluateEdges.clear();
    }
    Compact();
}

void MeshSimplifierImpl::Compact() {
    u32 vertexCount = 0;
    for (u32 i = 0; i < numVert; i++) {
        if (vertRefs[i] > 0) {
            if (i != vertexCount) verts[vertexCount] = verts[i];
            //重用作下标
            vertRefs[i] = vertexCount++;
        }
    }
    assert(vertexCount == remainingNumVert);

    u32 triCount = 0;
    for (u32 i = 0; i < numTri; i++) {
        if (!triRemoved[i]) {
            for (u32 k = 0; k < 3; k++) {
                indexes[triCount * 3 + k] = vertRefs[indexes[i * 3 + k]];
            }
            triCount++;
        }
    }
    assert(triCount == remainingNumTri);
}

MeshSimplifier::MeshSimplifier(vec3* verts, u32 numVert, u32* indexes, u32 numIndex) {
    impl = new MeshSimplifierImpl(verts, numVert, indexes, numIndex);
}

MeshSimplifier::~MeshSimplifier() {
    if (impl) delete (MeshSimplifierImpl*)impl;
}

void MeshSimplifier::LockPosition(vec3 p) {
    ((MeshSimplifierImpl*)impl)->LockPosition(p);
}

// bool MeshSimplifier::is_position_locked(vec3 p){
//     return ((MeshSimplifierImpl*)impl)->is_position_locked(p);
// }

void MeshSimplifier::Simplify(u32 targetNumTri) {
    ((MeshSimplifierImpl*)impl)->Simplify(targetNumTri);
}

u32 MeshSimplifier::RemainingNumVert() {
    return ((MeshSimplifierImpl*)impl)->remainingNumVert;
}

u32 MeshSimplifier::RemainingNumTri() {
    return ((MeshSimplifierImpl*)impl)->remainingNumTri;
}

f32 MeshSimplifier::MaxError() {
    return ((MeshSimplifierImpl*)impl)->maxError;
}