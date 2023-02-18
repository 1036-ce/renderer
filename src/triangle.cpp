#include <functional>
#include "triangle.h"

void Triangle::draw(IShader &shader, const mat4 &vp, DepthBuffer &zbuf, 
					ColorBuffer* color_buf, AA_Format aa_f) {
	for (int i = 0; i < 3; ++i) {
		scoord[i] = vp * verts[i];
		scoord[i] = scoord[i] / scoord[i][3];	

		// pts.xyz /= pts.w  pts.w = 1.0 / pts.w
		double t = 1.0 / verts[i].w;
		verts[i] = verts[i] / verts[i].w;
		verts[i].w = t;
	}

	int bbox_left   = std::min(scoord[0].x, std::min(scoord[1].x, scoord[2].x));
	int bbox_right  = std::max(scoord[0].x, std::max(scoord[1].x, scoord[2].x));
	int bbox_bottom = std::min(scoord[0].y, std::min(scoord[1].y, scoord[2].y));
	int bbox_top    = std::max(scoord[0].y, std::max(scoord[1].y, scoord[2].y));
	bbox_left   = std::max(0, bbox_left);
	bbox_right  = std::min(zbuf.width()  - 1, bbox_right);
	bbox_bottom = std::max(0, bbox_bottom);
	bbox_top    = std::min(zbuf.height() - 1, bbox_top);

	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;
	using Simpler = vector<tuple<int, depth_t, color_t>>(int, int, IShader&);
	std::function<Simpler> simpler;
	if (aa_f == AA_Format::NOAA) {
		assert(zbuf.simple_num() == 1);
		simpler = std::bind(&Triangle::noaa, this, _1, _2, 1, _3);
	}
	else if (aa_f == AA_Format::MSAA4) {
		assert(zbuf.simple_num() == 4);
		simpler = std::bind(&Triangle::msaa, this, _1, _2, 4, _3);
	}
	else if (aa_f == AA_Format::MSAA8) {
		assert(zbuf.simple_num() == 8);
		simpler = std::bind(&Triangle::msaa, this, _1, _2, 8, _3);
	}
	else if (aa_f == AA_Format::SSAA4) {
		assert(zbuf.simple_num() == 4);
		simpler = std::bind(&Triangle::ssaa, this, _1, _2, 4, _3);
	}

	vector<tuple<int, depth_t, color_t>> pack;
	for (int x = bbox_left; x <= bbox_right; ++x) {
		for (int y = bbox_bottom; y <= bbox_top; ++y) {
			pack = simpler(x, y, shader);
			for (int i = 0; i < pack.size(); ++i) {
				int   idx = std::get<0>(pack[i]);
				depth_t d = std::get<1>(pack[i]);
				if (d >= -1.0 && d <= 1.0 && d > zbuf.get(x, y, idx)) {
					color_t color = std::get<2>(pack[i]);
					if (color_buf && gl_blend) {
						color_t tmp = color_buf->get(x, y, idx);
						float alpha = (float)color[3] / 255.0;
						color = (color * alpha) + (tmp * (1 - alpha));
					}
					zbuf.set(x, y, idx, d);
					if (color_buf && color[3] != 0)	// if alpha == 0, ignore it
					// if (color_buf)	// if alpha == 0, ignore it
						color_buf->set(x, y, idx, color);
				}
			}
		}
	}
}

void Triangle::enable(const uint32_t & feature) {
	if (feature & GL_BLEND)
		gl_blend = true;
}

vec3 Triangle::baryentric(const vec2 &p) {
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

void Triangle::bar_corrent(vec3 &bar, double w) {
	bar = 1.0 / w * vec3(verts[0].w, verts[1].w, verts[2].w) * bar;
}

vector<tuple<int, depth_t, color_t>>
Triangle::noaa(int x, int y, int sample_num, IShader &shader) {
	vector<tuple<int, depth_t, color_t>> ret;
	vec3 bar  = baryentric(vec2(x + 0.5, y + 0.5));
	if (bar.x < 0 || bar.y < 0 || bar.z < 0)
		return ret;
	depth_t d = dot(vec3(verts[0].z, verts[1].z, verts[2].z), bar);
	double  w = dot(vec3(verts[0].w, verts[1].w, verts[2].w), bar);

	bar_corrent(bar, w);
	color_t color;
	shader.fragment(bar, color);
	ret.push_back(std::make_tuple(0, d, color));
	return ret;
}

vector<tuple<int, depth_t, color_t>>
Triangle::msaa(int x, int y, int sample_num, IShader &shader) {
	vector<vec2> offsets;
	if (sample_num == 4) {
		offsets = { {0.25, 0.25}, {0.75, 0.25}, {0.25, 0.75}, {0.75, 0.75} };
	}

	vector<tuple<int, depth_t, color_t>> ret;
	vector<int> tmp;
	vec2 target(0, 0);
	vec3 bar;
	for (int i = 0; i < sample_num; ++i) {
		vec2 t(x + offsets[i].x, y + offsets[i].y);
		bar = baryentric(t);
		if (bar.x>=0.0 && bar.y>=0.0 && bar.z>=0.0) {
			tmp.push_back(i);
			target = target + t;
		}
	}
	if (tmp.empty())
		return ret;
	target = target / tmp.size();
	bar = baryentric(target);
	depth_t d = dot(vec3(verts[0].z, verts[1].z, verts[2].z), bar);
	double w  = dot(vec3(verts[0].w, verts[1].w, verts[2].w), bar);

	bar_corrent(bar, w);
	color_t color;
	bool discard = shader.fragment(bar, color);
	if (discard) 
		color = TGAColor(0, 0, 0, 0);

	for (int idx: tmp) {
		ret.push_back(std::make_tuple(idx, d, color));
	}
	return ret;
}

vector<tuple<int, depth_t, color_t>>
Triangle::ssaa(int x, int y, int sample_num, IShader &shader) {
	vector<vec2> offsets;
	if (sample_num == 4) {
		offsets = { {0.25, 0.25}, {0.75, 0.25}, {0.25, 0.75}, {0.75, 0.75} };
	}

	vector<tuple<int, depth_t, color_t>> ret;
	vec3 bar;
	color_t color;
	for (int i = 0; i < sample_num; ++i) {
		bar = baryentric(vec2(x + offsets[i].x, y + offsets[i].y));
		if (bar.x>=0.0 && bar.y>=0.0 && bar.z>=0.0) {
			depth_t d = dot(vec3(verts[0].z, verts[1].z, verts[2].z), bar);
			double  w = dot(vec3(verts[0].w, verts[1].w, verts[2].w), bar);
			bar_corrent(bar, w);
			shader.fragment(bar, color);
			ret.push_back(std::make_tuple(i, d, color));
		}
	}
	return ret;
}
