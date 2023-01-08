#pragma once
#include <iostream>

namespace utils
{
	inline bool AreEqual(float a, float b, float epsilon = 0.00001f)
	{
		return abs(a - b) < epsilon;
	}

	
	inline bool RandomChange(int percentage)
	{
		const int randomValue{ rand() % 101 };
		if (randomValue < percentage) return true;
		else return false;
	}
}