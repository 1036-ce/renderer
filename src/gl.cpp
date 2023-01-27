#include "gl.h"

mat4 lookat(vec3 eye, vec3 center, vec3 up) {
	vec3 z = (eye - center).normalize();
	vec3 x = cross(up, z).normalize();
	vec3 y = cross(z, x).normalize();

	mat4 Minv = mat4::identity();
	mat4 Tr = mat4::identity();
	for (int i = 0; i < 3; ++i) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
		Minv[2][i] = z[i];
		// Tr[i][3] = -eye[i];
		Tr[i][3] = -center[i];
	}
	return Minv*Tr;
}

mat4 projection(float distance) {
	mat4 proj = mat4::identity();
	proj[3][2] = -1.0 / distance;
	return proj;
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
	if (std::abs(t.z) < 1e-2)
		return vec3(-1, 1, 1);
	float u = t.x / t.z;
	float v = t.y / t.z;
	return vec3(1 - u - v, u, v);
}

void triangle(vec4 pts[3], IShader &shader, TGAImage &image, TGAImage &zbuffer) {
	int bbox_left   = std::min(pts[0].x/pts[0][3], std::min(pts[1].x/pts[1][3], pts[2].x/pts[2][3]));
	int bbox_right  = std::max(pts[0].x/pts[0][3], std::max(pts[1].x/pts[1][3], pts[2].x/pts[2][3]));
	int bbox_bottom = std::min(pts[0].y/pts[0][3], std::min(pts[1].y/pts[1][3], pts[2].y/pts[2][3]));
	int bbox_top    = std::max(pts[0].y/pts[0][3], std::max(pts[1].y/pts[1][3], pts[2].y/pts[2][3]));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	TGAColor color;
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 tmp[3];
			for (int i = 0; i < 3; ++i)
				tmp[i] = proj<3>(pts[i] / pts[i][3]);
			vec3 bar = barycentric(tmp, vec3(x, y, 0));
			float z = pts[0][2] * bar[0] + pts[1][2] * bar[1] + pts[2][2] * bar[2];
			float w = pts[0][3] * bar[0] + pts[1][3] * bar[1] + pts[2][3] * bar[2];
			int frag_depth = std::max(0, std::min(255, int(z/w + 0.5)));
			if (bar.x<0 || bar.y<0 || bar.z<0 || zbuffer.get(x, y)[0]>frag_depth)
				continue;
			bool discard = shader.fragment(bar, color);
			if (!discard) {
				zbuffer.set(x, y, TGAColor(frag_depth));
				image.set(x, y, color);
			}
		}
	}

}

void triangle(vec3 *pts, TGAImage &image, TGAImage &zbuffer, TGAColor color) { 
	int bbox_left   = std::min(pts[0].x, std::min(pts[1].x, pts[2].x));
	int bbox_right  = std::max(pts[0].x, std::max(pts[1].x, pts[2].x));
	int bbox_bottom = std::min(pts[0].y, std::min(pts[1].y, pts[2].y));
	int bbox_top    = std::max(pts[0].y, std::max(pts[1].y, pts[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 b = barycentric(pts, vec3(x, y, 0));
			if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0)
				continue;
			int z = 0;
			for (int i = 0; i < 3; ++i)
				z += b[i] * pts[i][2];
			if (z > zbuffer.get(x, y)[0]) {
				zbuffer.set(x, y, TGAColor(z));
				image.set(x, y, color);
			}
		}
	}
}

void triangle(vec3 *pts, float *zbuf, TGAImage &image, TGAColor color) { 
	int bbox_left   = std::min(pts[0].x, std::min(pts[1].x, pts[2].x));
	int bbox_right  = std::max(pts[0].x, std::max(pts[1].x, pts[2].x));
	int bbox_bottom = std::min(pts[0].y, std::min(pts[1].y, pts[2].y));
	int bbox_top    = std::max(pts[0].y, std::max(pts[1].y, pts[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	int width = image.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 b = barycentric(pts, vec3(x, y, 0));
			if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0)
				continue;
			int z = 0;
			for (int i = 0; i < 3; ++i)
				z += b[i] * pts[i][2];
			if (z > zbuf[x + y * width]) {
				zbuf[x + y * width] = z;
				image.set(x, y, color);
			}
		}
	}
}

void triangle(vec3 pts[3], float *zbuf, TGAImage &image, vec2 uv_coords[3], TGAImage &diffuse, float intensity) { 
	int bbox_left   = std::min(pts[0].x, std::min(pts[1].x, pts[2].x));
	int bbox_right  = std::max(pts[0].x, std::max(pts[1].x, pts[2].x));
	int bbox_bottom = std::min(pts[0].y, std::min(pts[1].y, pts[2].y));
	int bbox_top    = std::max(pts[0].y, std::max(pts[1].y, pts[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	int width = image.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 b = barycentric(pts, vec3(x, y, 0));
			if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0)
				continue;
			int z = 0;
			for (int i = 0; i < 3; ++i)
				z += b[i] * pts[i][2];
			if (z > zbuf[x + y * width]) {
				zbuf[x + y * width] = z;
				vec2 uv;
				uv[0] = b[0] * uv_coords[0][0] + b[1] * uv_coords[1][0] + b[2] * uv_coords[2][0];
				uv[1] = b[0] * uv_coords[0][1] + b[1] * uv_coords[1][1] + b[2] * uv_coords[2][1];
				TGAColor color = diffuse.get(uv[0] * diffuse.width(), uv[1] * diffuse.height());
				for (int i = 3; i--; color[i] *= intensity);
				image.set(x, y, color);
			}
		}
	}
}

void triangle(vec3 pts[3], TGAImage &zbuf, TGAImage &image, vec2 uv_coords[3], TGAImage &diffuse, float intensity) {
	int bbox_left   = std::min(pts[0].x, std::min(pts[1].x, pts[2].x));
	int bbox_right  = std::max(pts[0].x, std::max(pts[1].x, pts[2].x));
	int bbox_bottom = std::min(pts[0].y, std::min(pts[1].y, pts[2].y));
	int bbox_top    = std::max(pts[0].y, std::max(pts[1].y, pts[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 b = barycentric(pts, vec3(x, y, 0));
			if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0)
				continue;
			int z = 0;
			for (int i = 0; i < 3; ++i)
				z += b[i] * pts[i][2];
			// if (z > zbuf[x + y * width]) {
			// 	zbuf[x + y * width] = z;
			int d = zbuf.get(x, y)[0];
			if (z > d) {
				zbuf.set(x, y, TGAColor(z));
				vec2 uv;
				uv[0] = b[0] * uv_coords[0][0] + b[1] * uv_coords[1][0] + b[2] * uv_coords[2][0];
				uv[1] = b[0] * uv_coords[0][1] + b[1] * uv_coords[1][1] + b[2] * uv_coords[2][1];
				TGAColor color = diffuse.get(uv[0] * diffuse.width(), uv[1] * diffuse.height());
				for (int i = 3; i--; color[i] *= intensity);
				image.set(x, y, color);
			}
		}
	}
}

void triangle(vec3 pts[3], TGAImage &zbuf, TGAImage &image, float intensity) {
	int bbox_left   = std::min(pts[0].x, std::min(pts[1].x, pts[2].x));
	int bbox_right  = std::max(pts[0].x, std::max(pts[1].x, pts[2].x));
	int bbox_bottom = std::min(pts[0].y, std::min(pts[1].y, pts[2].y));
	int bbox_top    = std::max(pts[0].y, std::max(pts[1].y, pts[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 b = barycentric(pts, vec3(x, y, 0));
			if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0)
				continue;
			int z = 0;
			for (int i = 0; i < 3; ++i)
				z += b[i] * pts[i][2];
			// if (z > zbuf[x + y * width]) {
			// 	zbuf[x + y * width] = z;
			int d = zbuf.get(x, y)[0];
			if (z > d) {
				zbuf.set(x, y, TGAColor(z));
				TGAColor color = TGAColor(255, 255, 255) * intensity;
				// TGAColor color = diffuse.get(uv[0] * diffuse.width(), uv[1] * diffuse.height());
				// for (int i = 3; i--; color[i] *= intensity);
				image.set(x, y, color);
			}
		}
	}
}

void triangle(vec3 pts[3], TGAImage &zbuf, TGAImage &image, float intensity[3]) {
	int bbox_left   = std::min(pts[0].x, std::min(pts[1].x, pts[2].x));
	int bbox_right  = std::max(pts[0].x, std::max(pts[1].x, pts[2].x));
	int bbox_bottom = std::min(pts[0].y, std::min(pts[1].y, pts[2].y));
	int bbox_top    = std::max(pts[0].y, std::max(pts[1].y, pts[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 b = barycentric(pts, vec3(x, y, 0));
			if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0)
				continue;
			int z = 0;
			for (int i = 0; i < 3; ++i)
				z += b[i] * pts[i][2];
			int d = zbuf.get(x, y)[0];
			float ity = 0.0;
			for (int i = 0; i < 3; ++i)
				ity += b[i] * intensity[i];
			if (z > d) {
				zbuf.set(x, y, TGAColor(z));
				TGAColor color = TGAColor(255, 255, 255) * ity;
				image.set(x, y, color);
			}
		}
	}
}

void triangle(const vec3 &v0, const vec3 &v1, const vec3 &v2, float *zbuf, TGAImage &image, TGAColor color) {
	int bbox_left   = std::min(v0.x, std::min(v1.x, v2.x));
	int bbox_right  = std::max(v0.x, std::max(v1.x, v2.x));
	int bbox_bottom = std::min(v0.y, std::min(v1.y, v2.y));
	int bbox_top    = std::max(v0.y, std::max(v1.y, v2.y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	int width = image.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 b = barycentric(v0, v1, v2, vec3(x, y, 0));
			if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0)
				continue;
			int z = b[0] * v0.z + b[1] * v1.z + b[2] * v2.z;

			if (z > zbuf[x + y * width]) {
				zbuf[x + y * width] = z;
				image.set(x, y, color);
			}
		}
	}
}
