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
	if (std::abs(t.z) < 1e-3) {
		return vec3(-1, 1, 1);
	}
	float u = t.x / t.z;
	float v = t.y / t.z;
	return vec3(1 - u - v, u, v);
}


vec3 barycentric(vec4 scoord[3], vec2 p) {
	vec3 t1 = vec3(scoord[1][0] - scoord[0][0], scoord[2][0] - scoord[0][0], scoord[0][0] - p[0]);
	vec3 t2 = vec3(scoord[1][1] - scoord[0][1], scoord[2][1] - scoord[0][1], scoord[0][1] - p[1]);
	vec3 t = cross(t1, t2);
	if (std::abs(t.z) < 1e-3) {
		return vec3(-1, 1, 1);
	}
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
	if (std::abs(t.z) < 1e-3)	// is not a triangle
		return vec3(-1, 1, 1);
	float u = t.x / t.z;
	float v = t.y / t.z;
	return vec3(1.0 - u - v, u, v);
}

std::vector<int> msaa(vec3 pts[3], float x, float y, vec3& v) {
	std::vector<vec2> offsets{
		{0.25, 0.25},
		{0.75, 0.25},
		{0.25, 0.75},
		{0.75, 0.75},
	};
	std::vector<int> ret;
	vec3 p(0, 0, 0);
	vec3 bar;
	for (int i = 0; i < 4; ++i) {
		vec3 tmp(x + offsets[i].x, y + offsets[i].y, 0);
		bar = barycentric(pts, tmp);
		if (bar.x>=0.0 && bar.y>=0.0 && bar.z>=0.0) {
			// v = bar;
			p = p + tmp;
			ret.push_back(i);
		}
	}
	if (!ret.empty()) {
		p = p / ret.size();
		v = barycentric(pts, p);
	}

	return ret;
}

void triangle(vec4 pts[3], IShader &shader, const mat4 &vp, float *zbuffer, Buffer<TGAColor> &color_buf) {

	auto bar_correct = [](vec3& bar, const vec4 pts[3], double w) {
		bar = 1.0 / w * vec3(pts[0].w, pts[1].w, pts[2].w) * bar;
	};
	// pts is clip space coord
	// scoord is screen space coord 
	vec4 scoord[3];		
	vec3 tmp[3];

	for (int i = 0; i < 3; ++i) {
		scoord[i] = vp * pts[i];
		scoord[i] = scoord[i] / scoord[i][3];	
		tmp[i] = vec3(scoord[i]);

		// pts.xyz /= pts.w  pts.w = 1.0 / pts.w
		double t = 1.0 / pts[i].w;
		pts[i] = pts[i] / pts[i].w;
		pts[i].w = t;
	}

	int bbox_left   = std::min(scoord[0].x, std::min(scoord[1].x, scoord[2].x));
	int bbox_right  = std::max(scoord[0].x, std::max(scoord[1].x, scoord[2].x));
	int bbox_bottom = std::min(scoord[0].y, std::min(scoord[1].y, scoord[2].y));
	int bbox_top    = std::max(scoord[0].y, std::max(scoord[1].y, scoord[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(color_buf.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(color_buf.height() - 1, bbox_top);

	TGAColor color;
	int width = color_buf.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 t;
			std::vector<int> v = msaa(tmp, x, y, t);
			if (v.empty())
				continue;

			vec3 bar = barycentric(tmp, vec3(x + 0.5, y + 0.5, 0));		// center of pixel(x, y) is (x + 0.5, y + 0.5)

			// if the center of pixel is not in triangle, we use simple dot's bar
			if (bar.x<0 || bar.y<0 || bar.z<0)	{
				bar = t;
			}

			double z = pts[0].z * bar[0] + pts[1].z * bar[1] + pts[2].z * bar[2];	// clip space z-value
			double w = pts[0].w * bar[0] + pts[1].w * bar[1] + pts[2].w * bar[2];	// view space 1.0/z-value

			if (z < -1.0 || z > 1.0 || z < zbuffer[y * width + x])
				continue;

			// convert to perspective correct barycentric
			// bar[0] = z / z_a * bar[0]
			// bar[1] = z / z_b * bar[1]
			// bar[2] = z / z_c * bar[2]
			// bar = 1.0 / w * vec3(pts[0].w, pts[1].w, pts[2].w) * bar;
			bar_correct(bar, pts, w);

			bool discard = shader.fragment(bar, color);
			if (!discard) {
				zbuffer[y * width + x] = z;
				for (int idx: v)
					color_buf.set(x, y, idx, color);
				// color_buf.set(x, y, 0, color);
			}
		}
	}


}

void triangle_msaa(vec4 pts[3], IShader & shader, const mat4 & vp,  DepthBuffer& zbuffer, ColorBuffer& color_buf) {
	auto bar_correct = [](vec3& bar, const vec4 pts[3], double w) {
		bar = 1.0 / w * vec3(pts[0].w, pts[1].w, pts[2].w) * bar;
	};

	// pts is clip space coord
	// scoord is screen space coord 
	vec4 scoord[3];		
	vec3 tmp[3];

	for (int i = 0; i < 3; ++i) {
		scoord[i] = vp * pts[i];
		scoord[i] = scoord[i] / scoord[i][3];	
		tmp[i] = vec3(scoord[i]);

		// pts.xyz /= pts.w  pts.w = 1.0 / pts.w
		double t = 1.0 / pts[i].w;
		pts[i] = pts[i] / pts[i].w;
		pts[i].w = t;
	}

	std::vector<vec2> offsets{
		{0.25, 0.25},
		{0.75, 0.25},
		{0.25, 0.75},
		{0.75, 0.75}
	};

	int bbox_left   = std::min(scoord[0].x, std::min(scoord[1].x, scoord[2].x));
	int bbox_right  = std::max(scoord[0].x, std::max(scoord[1].x, scoord[2].x));
	int bbox_bottom = std::min(scoord[0].y, std::min(scoord[1].y, scoord[2].y));
	int bbox_top    = std::max(scoord[0].y, std::max(scoord[1].y, scoord[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(color_buf.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(color_buf.height() - 1, bbox_top);

	TGAColor color;
	int width = color_buf.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			vec3 bar;
			std::vector v = msaa(tmp, x, y, bar);
			if (v.empty()) continue;

			double z = pts[0].z * bar[0] + pts[1].z * bar[1] + pts[2].z * bar[2]; // clip space z-value
			double w = pts[0].w * bar[0] + pts[1].w * bar[1] + pts[2].w * bar[2]; // view space 1.0/z-value

			bar_correct(bar, pts, w);

			bool discard = shader.fragment(bar, color);
			if (!discard) {
				for (auto idx: v) {
					if (z < -1.0 || z > 1.0 || z < zbuffer.get(x, y, idx))
						continue;
					zbuffer.set(x, y, idx, z);
					color_buf.set(x, y, idx, color);
				}
			}
		}
	}
}

void triangle_ssaa(vec4 pts[3], IShader & shader, const mat4 & vp,  DepthBuffer& zbuffer, ColorBuffer& color_buf)
{
	auto bar_correct = [](vec3& bar, const vec4 pts[3], double w) {
		bar = 1.0 / w * vec3(pts[0].w, pts[1].w, pts[2].w) * bar;
	};

	// pts is clip space coord
	// scoord is screen space coord 
	vec4 scoord[3];		
	vec3 tmp[3];

	for (int i = 0; i < 3; ++i) {
		scoord[i] = vp * pts[i];
		scoord[i] = scoord[i] / scoord[i][3];	
		tmp[i] = vec3(scoord[i]);

		// pts.xyz /= pts.w  pts.w = 1.0 / pts.w
		double t = 1.0 / pts[i].w;
		pts[i] = pts[i] / pts[i].w;
		pts[i].w = t;
	}

	std::vector<vec2> offsets{
		{0.25, 0.25},
		{0.75, 0.25},
		{0.25, 0.75},
		{0.75, 0.75}
	};

	int bbox_left   = std::min(scoord[0].x, std::min(scoord[1].x, scoord[2].x));
	int bbox_right  = std::max(scoord[0].x, std::max(scoord[1].x, scoord[2].x));
	int bbox_bottom = std::min(scoord[0].y, std::min(scoord[1].y, scoord[2].y));
	int bbox_top    = std::max(scoord[0].y, std::max(scoord[1].y, scoord[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(color_buf.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(color_buf.height() - 1, bbox_top);

	TGAColor color;
	int width = color_buf.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			for (int k = 0; k < 4; ++k) {
				vec3 bar = barycentric(tmp, vec3(x + offsets[k].x, y + offsets[k].y, 0));		// center of pixel(x, y) is (x + 0.5, y + 0.5)
				if (bar.x < 0 || bar.y < 0 || bar.z < 0) {
					continue;
				}

				double z = pts[0].z * bar[0] + pts[1].z * bar[1] + pts[2].z * bar[2]; // clip space z-value
				double w = pts[0].w * bar[0] + pts[1].w * bar[1] + pts[2].w * bar[2]; // view space 1.0/z-value

				// if (z < -1.0 || z > 1.0 || z < zbuffer[y * width + x])
					// continue;
				if (z < -1.0 || z > 1.0 || z < zbuffer.get(x, y, k))
					continue;

				// convert to perspective correct barycentric
				// bar[0] = z / z_a * bar[0]
				// bar[1] = z / z_b * bar[1]
				// bar[2] = z / z_c * bar[2]
				// bar = 1.0 / w * vec3(pts[0].w, pts[1].w, pts[2].w) * bar;
				bar_correct(bar, pts, w);

				bool discard = shader.fragment(bar, color);
				if (!discard) {
					zbuffer.set(x, y, k, z);
					color_buf.set(x, y, k, color);
					// color_buf.set(x, y, idx, color);
					// color_buf.set(x, y, 0, color);
				}
			}

		}
	}
}

void triangle(vec4 pts[3], IShader &shader, const mat4& vp, float *zbuffer, TGAImage &image) {

	// pts is clip space coord
	// scoord is screen space coord 
	vec4 scoord[3];		
	vec3 tmp[3];

	for (int i = 0; i < 3; ++i) {
		scoord[i] = vp * pts[i];
		scoord[i] = scoord[i] / scoord[i][3];	
		tmp[i] = vec3(scoord[i] / scoord[i][3]);

		// pts.xyz /= pts.w  pts.w = 1.0 / pts.w
		double t = 1.0 / pts[i].w;
		pts[i] = pts[i] / pts[i].w;
		pts[i].w = t;
	}

	int bbox_left   = std::min(scoord[0].x, std::min(scoord[1].x, scoord[2].x));
	int bbox_right  = std::max(scoord[0].x, std::max(scoord[1].x, scoord[2].x));
	int bbox_bottom = std::min(scoord[0].y, std::min(scoord[1].y, scoord[2].y));
	int bbox_top    = std::max(scoord[0].y, std::max(scoord[1].y, scoord[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(image.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(image.height() - 1, bbox_top);

	TGAColor color;
	int width = image.width();
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			// vec3 bar = barycentric(tmp, vec3(x + 0.5, y + 0.5, 0));		// center of pixel(x, y) is (x + 0.5, y + 0.5)
			vec3 bar = barycentric(scoord, vec2(x + 0.5, y + 0.5));		// center of pixel(x, y) is (x + 0.5, y + 0.5)
			if (bar.x<0 || bar.y<0 || bar.z<0)
				continue;

			double z = pts[0].z * bar[0] + pts[1].z * bar[1] + pts[2].z * bar[2];	// clip space z-value
			double w = pts[0].w * bar[0] + pts[1].w * bar[1] + pts[2].w * bar[2];	// view space 1.0/z-value

			if (z < -1.0 || z > 1.0 || z < zbuffer[y * width + x])
				continue;

			// convert to perspective correct barycentric
			// bar[0] = z / z_a * bar[0]
			// bar[1] = z / z_b * bar[1]
			// bar[2] = z / z_c * bar[2]
			bar = 1.0 / w * vec3(pts[0].w, pts[1].w, pts[2].w) * bar;

			bool discard = shader.fragment(bar, color);
			if (!discard) {
				zbuffer[y * width + x] = z;
				image.set(x, y, color);
			}
		}
	}

}


void get_zbuf_image(float *zbuf, TGAImage& image) {
	float low = std::numeric_limits<float>::max();
	float high = -low;
	int width = image.width(), height = image.height();
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			if (zbuf[j * width + i] != -std::numeric_limits<float>::max()) {
				low = std::min(low, zbuf[j * width + i]);
				high = std::max(high, zbuf[j * width + i]);
			}
		}
	}
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			float alpha = 255 / (high - low);
			float c = zbuf[j * width + i];
			image.set(i, j, TGAColor(alpha * (c - low)));
		}
	}
}
