#include "HashTable.h"

void HashTable::ResizeIndex(u32 _indexSize)
{
	u32* newIndex = new u32[_indexSize];
	memcpy(newIndex, nextIndex, sizeof(u32) * indexSize);
	delete[]nextIndex;
	nextIndex = newIndex;
	indexSize = _indexSize;
}

HashTable::HashTable(u32 _indexSize)
{
	hash = nullptr;
	nextIndex = nullptr;
	Resize(_indexSize);
}

HashTable::HashTable(u32 _hashSize, u32 _indexSize)
{
	hash = nullptr;
	nextIndex = nullptr;
	Resize(_hashSize, _indexSize);
}

HashTable::~HashTable()
{
	Free();
}

void HashTable::Resize(u32 _indexSize)
{
	Resize(LowerNearest2Power(_indexSize), _indexSize);
}

void HashTable::Resize(u32 _hashSize, u32 _indexSize)
{
	Free();
	assert((hashSize & (hashSize - 1)) == 0);

	hashSize = _hashSize;
	hashMask = hashSize - 1;
	indexSize = _indexSize;
	hash = new u32[hashSize];
	nextIndex = new u32[indexSize];
	memset(hash, 0xff, hashSize * 4);
}

void HashTable::Free()
{
	hashSize = 0;
	hashMask = 0;
	indexSize = 0;
	delete[]hash;
	hash = nullptr;
	delete[]nextIndex;
	nextIndex = nullptr;
}

void HashTable::Clear()
{
}

void HashTable::Add(u32 key, u32 index)
{
	if (index >= indexSize) {
		Resize(NextNearest2Power(index + 1));
	}
	key &= hashMask;
	nextIndex[index] = hash[key];
	hash[key] = index;
}

void HashTable::Remove(u32 key, u32 index)
{
	if (index >= indexSize)	return;
	key &= hashMask;
	if (hash[key] == index) {
		hash[key] = nextIndex[index];
	}
	else {
		for (u32 i = hash[key]; i != ~0u; i = nextIndex[i]) {
			if (nextIndex[i] == index) {
				nextIndex[i] = nextIndex[index];
				break;
			}
		}
	}
}


