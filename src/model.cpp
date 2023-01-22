#include <iostream>
#include <sstream>
#include "model.h"

Model::Model(const std::string filename) {
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail())
		return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			vec3 v;
			for (int i = 0; i < 3; ++i)
				iss >> v[i];
			verts.push_back(v);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			vec2 vt;
			for (int i = 0; i < 2; ++i)
				iss >> vt[i];
			tex_coord.push_back(vt);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			vec3 vn;
			for (int i = 0; i < 3; ++i)
				iss >> vn[i];
			norms.push_back(vn);
		}
		else if (!line.compare(0, 2, "f ")) {
			iss >> trash;
			int v, vt, vn;
			int cnt = 0;
			while (iss >> v >> trash >> vt >> trash >> vn) {
				facet_vrt.push_back(--v);
				facet_tex.push_back(--vt);
				facet_nrm.push_back(--vn);
				++cnt;
			}
			if (3 != cnt) {
				std::cerr << "Error: the obj file is supposed to be trangulated" << std::endl;
				in.close();
				return;
			}
		}
	}
	in.close();
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << " vt# " << tex_coord.size() << " vn# " << norms.size() << std::endl;
	load_texture(filename, "_diffuse.tga", 	  diffusemap );
	load_texture(filename, "_nm_tangent.tga", normalmap  );
	load_texture(filename, "_spec.tga", 	  specularmap);
}

int Model::nverts() const {
	return verts.size();
}

int Model::nfaces() const {
	return facet_vrt.size() / 3;
}

vec3 Model::normal(const int iface, const int nthvert) const {
	return norms[iface * 3 + nthvert];
}

vec3 Model::normal(const vec2 &uv) const {
	TGAColor c = normalmap.get(uv.x * normalmap.width(), uv.y * normalmap.height());
	return vec3{(double)c[2], (double)c[1], (double)c[0]} * 2.0 / 255.0 - vec3{1, 1, 1};
}

vec3 Model::vert(const int i) const {
	return verts[i];
}

vec3 Model::vert(const int iface, const int nthvert) const {
	return verts[facet_vrt[iface * 3 + nthvert]];
}

vec2 Model::uv(const int iface, const int nthvert) const {
	return tex_coord[facet_tex[iface * 3 + nthvert]];
}

void Model::load_texture(const std::string filename, const std::string suffix, TGAImage &img) {
	size_t dot = filename.find_last_of('.');
	if (dot == std::string::npos)
		return;
	std::string texfile = filename.substr(0, dot) + suffix;
	std::cerr << "texture file" << texfile << " loading " << (img.read_tga_file(texfile) ? "ok" : "failed") << std::endl;
}
