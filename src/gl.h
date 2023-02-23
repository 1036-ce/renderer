#pragma once
#include <optional>
#include "tgaimage.h"
#include "buffer.h"


class IShader {
public:
	virtual vec4 vertex(int iface, int nthvert) = 0;
	virtual std::optional<color_t> fragment(vec3 bar) = 0;
	virtual ~IShader() = default;
};


mat4 translate(const mat4& m, const vec3& v);

mat4 rotate(const mat4& m, const float& angle, const vec3& v);

mat4 scale(const mat4& m, const vec3& v);

// return View matrix
mat4 lookat(vec3 eye, vec3 center, vec3 up);

/**
 * we use right-hand coord, so '0>near>far'
 * [bottom, top] => [-1, 1];
 * [far, near]   => [-1, 1];
 * [left, right] => [-1, 1];
*/
mat4 orthographic(double bottom, double top, double near, double far, double left, double right);

// 0 > near > far
mat4 perspective(float fovY, float aspect, double near, double far);

mat4 viewport(int x, int y, int w, int h);

color_t texture(TGAImage *simpler, const vec2& uv);

float radius(float angle);

// return the area of the triangle
float area(vec3 pts[3]);

void line(int x0, int y0, int x1, int y1, TGAImage &image, color_t color);

/**
 * @brief compute relfect vector
 * 
 * @param n should be normalized
 * @param in should be normalized
 * @return vec3 
 */
vec3 reflect(const vec3& n, const vec3& in);

/**
 * @brief compute refract vector (may be not exist)
 * 
 * @param n should be normalized
 * @param in should be normalized
 * @return vec3 
 */
std::optional<vec3> refract(const vec3& N, const vec3& I, float etai, float etat);

/**
 * @brief compute fresnel equation
 * 
 * @return material's reflection ratio 
 * (1 - @return) is material's refraction ratio
 */
float fresnel(const vec3& I, const vec3& N, float etai, float etat);