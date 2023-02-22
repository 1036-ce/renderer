#pragma once
#include <string>
#include "global.h"
#include "geometry.h"
#include "tgaimage.h"

class Texture {
public:
	Texture() = default;
	Texture(TGAImage& image) : image(image) {}
	Texture(TGAImage&& image) : image(image) {}
	Texture(const std::string& path) {
		if (!image.read_tga_file(path)) {
			std::cerr << "load " + path + "failed" << std::endl;
		}
		else {
			image.flip_vertically();
		}
	}
	int width() { return image.width(); };
	int height() { return image.height(); };
	color_t sample(const vec2& uv);
	template<int nrows, int ncols> void convolute(const mat<nrows, ncols>& m);
private:
	TGAImage image;
};

template <int nrows, int ncols>
inline void Texture::convolute(const mat<nrows, ncols> &m) {
	image = image.convolute(m);
}
