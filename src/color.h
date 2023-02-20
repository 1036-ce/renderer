#pragma once

class Color {
public:
	float r, g, b, a;
	Color() = default;
	Color(float _r) : r(_r) {}
	Color(float _r, float _g, float _b, float _a = 1.0) 
		: r(_r), g(_g), b(_b), a(_a) {}
	float operator[](int idx);
private:
};

Color operator*(const Color& c, float intensity);
Color operator+(const Color& lhs, const Color& rhs);