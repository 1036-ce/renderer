#pragma once
#include <vector>
#include <string>
#include <optional>
#include "geometry.h"
#include "tgaimage.h"
#include "gl.h"
#include "triangle.h"

/**
 vertex : v {x} {y} {z} [w]	; w is optional and defaults to 1.0
 		example : v 0.123 0.234 0.345 1.0
 texture coordinates : vt {u} [v] [w] vary between 0 and 1. v, w defaults to 0
		example : vt 0.500 0.123 0.231
 vertex normals : vn {x} {y} {z}
 		example : vn  0.001 0.482 -0.876
 face element : f {v}/[vt]/[vn]
		example : f 6/4/1 3/5/3 7/6/5

*/
class Model {
public:
	Model(const std::string filename); 
	void draw(IShader& shader, const mat4& vp, DepthBuffer& depth_buf, 
			  ColorBuffer *color_buf, Triangle::AA_Format aa_f = Triangle::NOAA);
	void enable(const uint16_t& feature);
	int nverts() const;
	int nfaces() const;
	vec3 normal(const int iface, const int nthvert) const; 	// per triangle corner normal vertex
	vec3 normal(const vec2 &uv) const; 						// fetch the normal from the normal map texture
	vec3 vert(const int i) const;
	vec3 vert(const int iface, const int nthvert) const;
	vec2 uv(const int iface, const int nthvert) const;
	color_t diffuse(const vec2& uv) const;
	float specular(const vec2& uv) const;
private:
	// load texture	
	void load_texture(const std::string filename, const std::string suffix, TGAImage& img);
	void gen_normal();

	bool gl_blend = false;
	std::vector<vec3> verts{};		// array of vertices
	std::vector<vec2> tex_coord{}; 	// per-vertex array of tex coords
	std::vector<vec3> norms{};		// per-vertex array of notmal vectors
	std::vector<int> facet_vrt{};
	std::vector<int> facet_tex{};	// per-triangle indices in the above arrays
	std::vector<int> facet_nrm{};
	TGAImage diffusemap{};			// diffuse color texture
	TGAImage normalmap{};			// normal map texture
	TGAImage specularmap{};			// specular map texture
};
