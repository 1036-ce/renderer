#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0  , 255, 0,   255);

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

void draw() {
	Model *model = new Model("/home/zhaosiqi/workspace/Projects/renderer/obj/african_head/african_head.obj");

	int width = 2000, height = 2000;
	vec3 light_dir(0, 0, -1);
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage diffuse = model->diffuse();
	diffuse.flip_vertically();
	float *zbuf = new float[width * height];
	for (int i = width * height; i--; zbuf[i] = -std::numeric_limits<float>::max());
	for (int i = 0; i < model->nfaces(); ++i) {
		vec3 screen_coords[3];
		vec3 world_coords[3];
		for (int j = 0; j < 3; ++j) {
			vec3 v = model->vert(i, j);
			screen_coords[j].x = width  * (v.x + 1.0) / 2.0;
			screen_coords[j].y = height * (v.y + 1.0) / 2.0;
			screen_coords[j].z = v.z;
			world_coords[j] = v;
		}
		vec3 n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]).normalize();
		float intensity = n * light_dir;
		vec2 uv[3];
		for (int j = 0; j < 3; ++j)
			uv[j] = model->uv(i, j);
		if (intensity > 0)
			triangle(screen_coords, zbuf, image, uv, diffuse, intensity);
		// 	triangle(screen_coords, zbuf, image, TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255));
	}
	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png /home/zhaosiqi/workspace/Projects/renderer");
}

int main() {
	draw();
	return 0;
}
