#pragma once
#include <cstdint>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
#include "geometry.h"

#pragma pack(push, 1)
struct TGAHeader
{
	std::uint8_t  id_length{};
	std::uint8_t  color_map_type{};
	std::uint8_t  image_type{};
	std::uint16_t color_map_origin{};
	std::uint16_t color_map_length{};
	std::uint8_t  color_map_entry_size{};
	std::uint16_t x_origin{};
	std::uint16_t y_origin{};
	std::uint16_t width{};
	std::uint16_t height{};
	std::uint8_t  bits_per_pixel{};
	std::uint8_t  image_descriptor{};
};
#pragma pack(pop)

struct TGAColor
{
	std::uint8_t bgra[4] = {0, 0, 0, 0};
	std::uint8_t bytes_per_pixel = 0;

	TGAColor() = default;
	TGAColor(const std::uint8_t R, const std::uint8_t G, const std::uint8_t B, const std::uint8_t A = 255)
		: bgra{B,G,R,A}, bytes_per_pixel(4) { }
	TGAColor(const std::uint8_t *p, const std::uint8_t bytes_per_pixel) : bytes_per_pixel(bytes_per_pixel) {
		for (std::uint8_t i = 0; i < bytes_per_pixel; ++i)
			bgra[i] = p[i];
	}
	TGAColor(const std::uint8_t v): bgra{v, 0, 0, 0}, bytes_per_pixel(1) { }

	std::uint8_t& operator[](const int i) { return bgra[i]; }

};


TGAColor operator*(const TGAColor& c, float intensity);
TGAColor operator+(const TGAColor& lhs, const TGAColor& rhs);
//  {
// }

class TGAImage
{
public:
	enum Format {GRAYSCALE = 1, RGB = 3, RGBA = 4};

	TGAImage() = default;
	TGAImage(const int width, const int height, const int bytes_per_pixel);
	bool  read_tga_file(const std::string filename);
	bool write_tga_file(const std::string filename, const bool vflip = true, const bool rle = true) const;
	void flip_horizontally();
	void flip_vertically();
	TGAColor get(const int x, const int y) const;
	void 	 set(const int x, const int y, const TGAColor& c);
	// nrows and ncols should be odd number
	template<int nrows, int ncols> TGAImage convolute(const mat<nrows, ncols>& m);
	int width() const;
	int height() const; 
	void clear();
private:
	bool   load_rle_data(std::ifstream& in);
	bool unload_rle_data(std::ofstream& out) const;

	int w  = 0;
	int h = 0;
	int bpp = 0;	// bytes per pixel
	std::vector<std::uint8_t> data = {};
};

template <int nrows, int ncols>
inline TGAImage TGAImage::convolute(const mat<nrows, ncols> &m) {
	int left   = ncols / 2, right = w - ncols + 1;
	int bottom = nrows / 2, top = h - nrows + 1;

	auto in_box = [&](int x, int y) -> bool {
		return x>=left && x<=right && y>=bottom && y<=top;
	};

	TGAImage ret(w, h, bpp);

	vec2 center(ncols / 2, nrows / 2);
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			if (!in_box(x, y))
				ret.set(x, y, this->get(x, y));
			else {
				TGAColor tmp;
				for (int i = 0; i < ncols; ++i) {
					for (int j = 0; j < nrows; ++j) {
						vec2 pos(x + i - center.x, y + j - center.y);
						tmp = tmp + this->get(pos.x, pos.y) * m[i][j];
					}
				}
				ret.set(x, y, tmp);
			}
		}
	}
	return ret;
}
