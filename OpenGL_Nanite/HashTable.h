#pragma once
#include <iostream>
#include <vector>
#include "Types.h"
#include <assert.h>
using namespace std;
//MurMurHashÀ„∑®

inline u32 MurMurAdd(u32 hash, u32 element) {
	element *= 0xcc9e2d51;
	element = (element << 15) | (element >> (32 - 15));
	element *= 0x1b873593;

	hash ^= element;
	hash = (hash << 13) | (hash >> (32 - 13));
	hash = hash * 5 + 0xe6546b64;
	return hash;
}

inline u32 MurMurMix(u32 hash) {
	hash ^= hash >> 16;
	hash *= 0x85ebca6b;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35;
	hash ^= hash >> 16;
	return hash;
}

inline u32 LowerNearest2Power(u32 x) {
	while (x & (x - 1)) {
		x ^= (x & (0 - x));
	}
	return x;
}

inline u32 NextNearest2Power(u32 x) {
	while (x & (x - 1)) {
		x ^= (x & (0 - x));
	}
	return x == 0 ? 1 : (x << 1);
}


class HashTable {
private:
	u32 hashSize;
	u32 hashMask;
	u32 indexSize;
	u32* hash;
	u32* nextIndex;
	void ResizeIndex(u32 _indexSize);
public:
	HashTable(u32 _indexSize = 0);
	HashTable(u32 _hashSize, u32 _indexSize);
	~HashTable();

	void Resize(u32 _indexSize);
	void Resize(u32 _hashSize, u32 _indexSize);

	void Free();
	void Clear();
	void Add(u32 key, u32 index);
	void Remove(u32 key, u32 index);

	struct Container {
		u32 index;
		u32* next;
		struct iterator {
			u32 index;
			u32* next;
			void operator++() { index = next[index]; }
			bool operator!=(const iterator& b)const { return index != b.index; }
			u32 operator*() { return index; }
		};
		iterator begin() { return iterator{ index, next }; }
		iterator end() { return iterator{ ~0u, nullptr }; }
	};

	Container operator[](u32 key) {
		if (hashSize == 0 || indexSize == 0) {
			return Container{~0u, nullptr};
		}
		key &= hashMask;
		return Container{ hash[key], nextIndex };
	}
};