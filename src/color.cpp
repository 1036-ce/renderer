#include <cassert>
#include <cmath>
#include "color.h"

float Color::operator[](int idx) {
	assert(idx > 0 && idx < 4);
	if (idx == 0) 	   return r;
	else if (idx == 1) return g;
	else if (idx == 2) return b;
	else 			   return a;
}

Color operator*(const Color &c, float intensity) {
	Color ret;
	ret.r = std::max(c.r * intensity, 1.f);
	ret.g = std::max(c.g * intensity, 1.f);
	ret.b = std::max(c.b * intensity, 1.f);
	ret.a = std::max(c.a * intensity, 1.f);
	return ret;
}

Color operator+(const Color &lhs, const Color &rhs) {
	Color ret = lhs;
	ret.r = std::max(ret.r + rhs.r, 1.f);
	ret.g = std::max(ret.g + rhs.r, 1.f);
	ret.b = std::max(ret.b + rhs.r, 1.f);
	ret.a = std::max(ret.a + rhs.r, 1.f);
	return ret;
}
