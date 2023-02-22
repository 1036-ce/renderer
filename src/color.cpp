#include <cassert>
#include <cmath>
#include "color.h"

float& Color::operator[](int idx) {
	assert(idx >= 0 && idx < 4);
	if (idx == 0) 	   return b;
	else if (idx == 1) return g;
	else if (idx == 2) return r;
	else 			   return a;
}

Color operator*(const Color &c, float intensity) {
	assert(intensity >= 0.0 && intensity <= 1.0);
	Color ret;
	ret.r = std::min(c.r * intensity, 1.f);
	ret.g = std::min(c.g * intensity, 1.f);
	ret.b = std::min(c.b * intensity, 1.f);
	ret.a = std::min(c.a * intensity, 1.f);
	return ret;
}

Color operator+(const Color &lhs, const Color &rhs) {
	Color ret;
	ret.r = std::min(lhs.r + rhs.r, 1.f);
	ret.g = std::min(lhs.g + rhs.g, 1.f);
	ret.b = std::min(lhs.b + rhs.b, 1.f);
	ret.a = std::min(lhs.a + rhs.a, 1.f);
	return ret;
}
