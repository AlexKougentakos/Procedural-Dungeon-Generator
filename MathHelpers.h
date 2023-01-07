#pragma once
#include <iostream>

namespace utils
{
	inline bool AreEqual(float a, float b, float epsilon = 0.00001f)
	{
		return abs(a - b) < epsilon;
	}

	
	inline bool RandomChange(float percentage)
	{
		//Cap the precision to 2 floating points
		constexpr int accuracy{ -2 };
		percentage = round(powf(percentage * 10, -accuracy)) / powf(10, -accuracy);


	}
}