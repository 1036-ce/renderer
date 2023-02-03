#include "tgaimage.h"
#include "model.h"

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

float radius(float angle);

class IShader {
public:
	virtual vec4 vertex(int iface, int nthvert) = 0;
	virtual bool fragment(vec3 bar, TGAColor &color) = 0;
	virtual ~IShader() = default;
};


// 重心坐标
// P = (1 - u - v)A + uB + vC
// @return vec3(1-u-v, u, v);
vec3 barycentric(vec3 *pts, vec3 p);

// 重心坐标
// P = (1 - u - v)A + uB + vC
// @return vec3(1-u-v, u, v);
vec3 barycentric(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& p);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);

void triangle(vec4 pts[3], IShader &shader, TGAImage &image, float *zbuffer);

void get_zbuf_image(float *zbuf, int width, int height, TGAImage& image);