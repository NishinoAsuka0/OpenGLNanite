#pragma once
#include <vector>
#include "Types.h"
#include "assert.h"
#include <iostream>
using namespace std;

class Heap {
    u32 heapSize;
    u32 numIndex;
    u32* heap;
    f32* keys;
    u32* heapIndexes;

    void ShiftUp(u32 i);
    void ShiftDown(u32 i);
public:
    Heap();
    Heap(u32 _numIndex);
    ~Heap() { Free(); }

    void Free() {
        heapSize = 0, numIndex = 0;
        delete[] heap;
        delete[] keys;
        delete[] heapIndexes;
        heap = nullptr, keys = nullptr, heapIndexes = nullptr;
    }
    void Resize(u32 _numIndex);

    f32 GetKey(u32 idx);
    void Clear();
    bool Empty() { return heapSize == 0; }
    bool IsPresent(u32 idx) { return heapIndexes[idx] != ~0u; }
    u32 Top();
    void Pop();
    void Add(f32 key, u32 idx);
    void Update(f32 key, u32 idx);
    void Remove(u32 idx);
};

