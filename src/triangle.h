#pragma once
#include <vector>
#include <utility>
#include <tuple>
#include "tgaimage.h"
#include "geometry.h"
#include "gl.h"

using std::vector;
using std::tuple;

using pack_t = vector<tuple<int, depth_t, std::optional<color_t>>>;

class Triangle
{
public:
	enum AA_Format { NOAA = 1, MSAA4 = 4, MSAA8 = 8, MSAA16 = 16 };
	void draw(IShader& shader, const mat4 & vp, DepthBuffer& zbuf, 
			  ColorBuffer* color_buf, AA_Format aa_f = AA_Format::NOAA);
	void enable(const uint32_t& feature);
	Triangle() = default;
	Triangle(vec4 pts[3]) { for (int i = 3; i--; verts[i] = pts[i]); }
	Triangle(vec4 A, vec4 B, vec4 C) {
		verts[0] = A;
		verts[1] = B;
		verts[2] = C;
	}
	~Triangle() = default;
private:
	vec3 baryentric(const vec2& p);
	void bar_corrent(vec3& bar, double w);
	bool gl_blend = false;
	// vector<pair<int, TGAColor>> msaa(int x, int y, int sample_num, IShader& shader);
	// vector<pair<int, TGAColor>> ssaa(int x, int y, int sample_num, IShader& shader);
	pack_t noaa(int x, int y, int sample_num, IShader& shader);
	pack_t msaa(int x, int y, int sample_num, IShader& shader);
	// pack_t ssaa(int x, int y, int sample_num, IShader& shader);

	vec4 verts[3];	// vertexs of triangle in clip space
	vec4 scoord[3];	// vertex of triangle in screen space
};