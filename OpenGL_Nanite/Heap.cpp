#include "Heap.h"

Heap::Heap() {
    heap = nullptr, keys = nullptr, heapIndexes = nullptr;
    heapSize = 0, numIndex = 0;
}

Heap::Heap(u32 _numIndex) {
    heapSize = 0;
    numIndex = _numIndex;
    heap = new u32[numIndex];
    keys = new f32[numIndex];
    heapIndexes = new u32[numIndex];
    memset(heapIndexes, 0xff, numIndex * sizeof(u32));
}

void Heap::Resize(u32 _numIndex) {
    Free();
    heapSize = 0;
    numIndex = _numIndex;
    heap = new u32[numIndex];
    keys = new f32[numIndex];
    heapIndexes = new u32[numIndex];
    memset(heapIndexes, 0xff, numIndex * sizeof(u32));
}

void Heap::ShiftUp(u32 i) {
    u32 idx = heap[i];
    u32 fa = (i - 1) >> 1;
    while (i > 0 && keys[idx] < keys[heap[fa]]) {
        heap[i] = heap[fa];
        heapIndexes[heap[i]] = i;
        i = fa, fa = (i - 1) >> 1;
    }
    heap[i] = idx;
    heapIndexes[heap[i]] = i;
}

void Heap::ShiftDown(u32 i) {
    u32 idx = heap[i];
    u32 ls = (i << 1) + 1;
    u32 rs = ls + 1;
    while (ls < heapSize) {
        u32 t = ls;
        if (rs < heapSize && keys[heap[rs]] < keys[heap[ls]])
            t = rs;
        if (keys[heap[t]] < keys[idx]) {
            heap[i] = heap[t];
            heapIndexes[heap[i]] = i;
            i = t, ls = (i << 1) + 1, rs = ls + 1;
        }
        else break;
    }
    heap[i] = idx;
    heapIndexes[heap[i]] = i;
}

void Heap::Clear() {
    heapSize = 0;
    memset(heapIndexes, 0xff, numIndex * sizeof(u32));
}

u32 Heap::Top() {
    assert(heapSize > 0);
    return heap[0];
}

void Heap::Pop() {
    assert(heapSize > 0);

    u32 idx = heap[0];
    heap[0] = heap[--heapSize];
    heapIndexes[heap[0]] = 0;
    heapIndexes[idx] = ~0u;
    ShiftDown(0);
}

void Heap::Add(f32 key, u32 idx) {
    assert(!IsPresent(idx));

    u32 i = heapSize++;
    heap[i] = idx;
    keys[idx] = key;
    heapIndexes[idx] = i;
    ShiftUp(i);
}

void Heap::Update(f32 key, u32 idx) {
    keys[idx] = key;
    u32 i = heapIndexes[idx];
    if (i > 0 && key < keys[heap[(i - 1) >> 1]]) ShiftUp(i);
    else ShiftDown(i);
}

void Heap::Remove(u32 idx) {
    //if(!is_present(idx)) return;
    assert(IsPresent(idx));

    f32 key = keys[idx];
    u32 i = heapIndexes[idx];

    if (i == heapSize - 1) {
        --heapSize;
        heapIndexes[idx] = ~0u;
        return;
    }

    heap[i] = heap[--heapSize];
    heapIndexes[heap[i]] = i;
    heapIndexes[idx] = ~0u;
    if (key < keys[heap[i]]) ShiftDown(i);
    else ShiftUp(i);
}

f32 Heap::GetKey(u32 idx) {
    return keys[idx];
}