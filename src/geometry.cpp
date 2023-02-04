#include "geometry.h"

double dot(const vec2 &v1, const vec2 &v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

double dot(const vec3 &v1, const vec3 &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

double dot(const vec4 &v1, const vec4 &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

vec3 cross(const vec3 &v1, const vec3 &v2) {
	return vec3{v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x};
}

vec3 reflect(const vec3& n, const vec3& in) {
	return ((-2 * n * in) * n + in).normalize();
}
