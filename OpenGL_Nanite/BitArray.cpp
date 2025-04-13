#include "BitArray.h"

BitArray::BitArray(u32 size)
{
	bits = vector<u32>((size + 31) / 32);
}

void BitArray::Resize(u32 size)
{
	bits = vector<u32>((size + 31) / 32);
}


void BitArray::SetFalse(u32 index)
{
	u32 x = index >> 5;
	u32 y = index & 31;
	bits[x] &= ~(1 << y);
}

void BitArray::SetTrue(u32 index)
{
	u32 x = index >> 5;
	u32 y = index & 31;
	bits[x] |= (1 << y);
}

bool BitArray::operator[](u32 index)
{
	u32 x = index >> 5;
	u32 y = index & 31;
	return (bool)((bits[x] >> y) & 1);
}
