#pragma once
#include <vector>
#include <utility>
#include <tuple>
#include "tgaimage.h"
#include "geometry.h"
#include "gl.h"

using std::vector;
using std::tuple;

using depth_t = float;
using color_t = TGAColor;

class Triangle
{
public:
	enum AA_Format { NOAA, MSAA4, MSAA8, SSAA4 };
	void draw(IShader& shader, const mat4 & vp, DepthBuffer& zbuf, ColorBuffer& color_buf, AA_Format aa_f = AA_Format::NOAA);
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
	// vector<pair<int, TGAColor>> msaa(int x, int y, int sample_num, IShader& shader);
	// vector<pair<int, TGAColor>> ssaa(int x, int y, int sample_num, IShader& shader);
	vector<tuple<int, depth_t, color_t>> noaa(int x, int y, int sample_num, IShader& shader);
	vector<tuple<int, depth_t, color_t>> msaa(int x, int y, int sample_num, IShader& shader);
	vector<tuple<int, depth_t, color_t>> ssaa(int x, int y, int sample_num, IShader& shader);

	vec4 verts[3];	// vertexs of triangle in clip space
	vec4 scoord[3];	// vertex of triangle in screen space
};