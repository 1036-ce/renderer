#include <vector>
#include <utility>
#include "gl.h"

mat4 translate(const mat4 &m, const vec3 &v) {
	mat4 ret = mat4::identity();
	ret[0][3] = v.x;
	ret[1][3] = v.y;
	ret[2][3] = v.z;
	return ret * m;
}

mat4 rotate(const mat4 &m, const float &angle, const vec3 &v) {
	float s  = sin(angle);
	float c = cos(angle);
	vec3 axis = v.normalize();
	vec3 tmp  = (1.0 - c) * axis;
	mat4 ret = mat4::identity();

	ret[0][0] = tmp[0] * axis[0] + c;
	ret[0][1] = tmp[0] * axis[1] - s * axis[2];
	ret[0][2] = tmp[0] * axis[2] + s * axis[1];

	ret[1][0] = tmp[1] * axis[0] + s * axis[2];
	ret[1][1] = tmp[1] * axis[1] + c;
	ret[1][2] = tmp[1] * axis[2] - s * axis[0];

	ret[2][0] = tmp[2] * axis[0] - s * axis[1];
	ret[2][1] = tmp[2] * axis[1] + s * axis[0];
	ret[2][2] = tmp[2] * axis[2] + c;
	return ret * m;
}

mat4 scale(const mat4 &m, const vec3 &v) {
	mat4 ret = mat4::identity();
	ret[0][0] = v.x;
	ret[1][1] = v.y;
	ret[2][2] = v.z;
	return ret * m;
}

mat4 lookat(vec3 eye, vec3 center, vec3 up) {
	vec3 z = (eye - center).normalize();
	vec3 x = cross(up, z).normalize();
	// vec3 x = cross(z, up).normalize();
	vec3 y = cross(z, x).normalize();

	mat4 rotation = mat4::identity();
	mat4 translation = mat4::identity();
	rotation[0] = vec4(x, 0);
	rotation[1] = vec4(y, 0);
	rotation[2] = vec4(z, 0);
	translation.set_col(3, vec4(-eye, 1));
	return rotation*translation;
}

mat4 orthographic(double bottom, double top, double near, double far, double left, double right) {
	mat4 ret = mat4::identity();
	ret[0][0] = 2.0 / (right - left);
	ret[1][1] = 2.0 / (top   - bottom);
	ret[2][2] = 2.0 / (near  - far);

	ret[0][3] = (left + right) / (left - right);
	ret[1][3] = (bottom + top) / (bottom - top);
	ret[2][3] = (far   + near) / (far   - near);
	return ret;
}

mat4 perspective(float fovY, float aspect, double near, double far) {
	mat4 ret = mat4::identity();
	/**
	 * 	tan(fovY/2) = -1 * (top / near)	near < 0
	 */
	float t = -tan(fovY * 0.5);
	ret[0][0] = 1.0 / (t * aspect);
	ret[1][1] = 1.0 / t;
	ret[2][2] = (near + far) / (near - far);
	ret[2][3] = 2 * near * far / (far - near);
	ret[3][2] = 1;
	ret[3][3] = 0;
	return ret;
}

float radius(float angle) {
	// angel * pi / 180
	return angle * 0.01745329251994329576923690768489;
}

mat4 viewport(int x, int y, int w, int h) {
	const double depth = 255.0;
	mat4 m = mat4::identity();
	m[0][0] = w/2.0, m[0][3] = x + w/2.0;
	m[1][1] = h/2.0, m[1][3] = y + h/2.0;
	m[2][2] = depth/2.0, m[2][3] = depth/2.0;
	return m;
}

TGAColor texture(TGAImage *simpler, const vec2 &uv) {
	int x = simpler->width()  * uv.x;
	int y = simpler->height() * uv.y;
	return simpler->get(x, y);
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		steep = true;
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	for (int x = x0; x <= x1; ++x) {
		float t = (float)(x - x0) / (float)(x1 - x0);
		int y = y0 + t * (y1 - y0);
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
	}
}