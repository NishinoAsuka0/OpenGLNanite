#pragma once
#include <vector>
#include "Types.h"
#include <iostream>
using namespace std;

class BitArray {
private:
	vector<u32>bits;
public:
	BitArray() {
		bits = vector<u32>(0);
	}
	BitArray(u32 size);
	void Resize(u32 size);
	void SetFalse(u32 index);
	void SetTrue(u32 index);
	bool operator[](u32 index);
};
