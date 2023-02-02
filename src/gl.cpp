#include "gl.h"

mat4 lookat(vec3 eye, vec3 center, vec3 up) {
	vec3 z = (eye - center).normalize();
	vec3 x = cross(up, z).normalize();
	// vec3 x = cross(z, up).normalize();
	vec3 y = cross(z, x).normalize();

	mat4 Minv = mat4::identity();
	mat4 Tr = mat4::identity();
	for (int i = 0; i < 3; ++i) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
		Minv[2][i] = z[i];

		Tr[i][3] = -eye[i];
		// Tr[i][3] = -center[i];
	}
	return Minv*Tr;
}

mat4 projection(float distance) {
	mat4 proj = mat4::identity();
	proj[3][2] = -1.0 / distance;
	return proj;
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
	// ret[2][2] = 1 + far / near;
	// ret[2][3] = -far;
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

// 重心坐标
// P = (1 - u - v)A + uB + vC
// @return vec3(1-u-v, u, v);
vec3 barycentric(vec3 *pts, vec3 p) {
	vec3 t1 = vec3(pts[1][0] - pts[0][0], pts[2][0] - pts[0][0], pts[0][0] - p[0]);
	vec3 t2 = vec3(pts[1][1] - pts[0][1], pts[2][1] - pts[0][1], pts[0][1] - p[1]);
	vec3 t = cross(t1, t2);
	if (std::abs(t.z) < 1e-2)
		return vec3(-1, 1, 1);
	float u = t.x / t.z;
	float v = t.y / t.z;
	return vec3(1 - u - v, u, v);
}

// 重心坐标
// P = (1 - u - v)A + uB + vC
// @return vec3(1-u-v, u, v);
vec3 barycentric(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& p) {
	vec3 t1 = vec3(v1.x - v0.x, v2.x - v0.x, v0.x - p.x);
	vec3 t2 = vec3(v1.y - v0.y, v2.y - v0.y, v0.y - p.y);
	vec3 t = cross(t1, t2);
	if (std::abs(t.z) < 1e-2)	// is not a triangle
		return vec3(-1, 1, 1);
	float u = t.x / t.z;
	float v = t.y / t.z;
	return vec3(1 - u - v, u, v);
}

void triangle(vec4 pts[3], IShader &shader, TGAImage &image, float *zbuffer) {
	int bbox_left   = std::min(pts[0].x/pts[0].w, std::min(pts[1].x/pts[1].w, pts[2].x/pts[2].w));
	int bbox_right  = std::max(pts[0].x/pts[0].w, std::max(pts[1].x/pts[1].w, pts[2].x/pts[2].w));
	int bbox_bottom = std::min(pts[0].y/pts[0].w, std::min(pts[1].y/pts[1].w, pts[2].y/pts[2].w));
	int bbox_top    = std::max(pts[0].y/pts[0].w, std::max(pts[1].y/pts[1].w, pts[2].y/pts[2].w));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	vec3 tmp[3];
	for (int i = 0; i < 3; ++i)
		tmp[i] = proj<3>(pts[i] / pts[i][3]);
	TGAColor color;
	int width = image.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 bar = barycentric(tmp, vec3(x + 0.5, y + 0.5, 0));		// a center of pixel(x, y) is (x + 0.5, y + 0.5)
			float z = pts[0][2] * bar[0] + pts[1][2] * bar[1] + pts[2][2] * bar[2];
			float w = pts[0][3] * bar[0] + pts[1][3] * bar[1] + pts[2][3] * bar[2];
			float frag_depth = z/w;
			if (bar.x<0 || bar.y<0 || bar.z<0 || zbuffer[y * width + x]>frag_depth)
				continue;
			// if (bar.x<0 || bar.y<0 || bar.z<0 || zbuffer.get(x, y)[0]>frag_depth)
			// 	continue;
			bool discard = shader.fragment(bar, color);
			if (!discard) {
				// zbuffer.set(x, y, TGAColor(frag_depth));
				zbuffer[y * width + x] = frag_depth;
				image.set(x, y, color);
			}
		}
	}

}

