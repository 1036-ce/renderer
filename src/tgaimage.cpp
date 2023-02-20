#include <iostream>
#include <cstring>
#include "tgaimage.h"

TGAImage::TGAImage(const int width, const int height, const int bytes_per_pixel)
	: w(width), h(height), bpp(bytes_per_pixel)
	,data(width * height * bytes_per_pixel, 0) { }

bool TGAImage::read_tga_file(const std::string filename) { 
	std::ifstream in;
	in.open(filename, std::ios::binary);
	if (!in.is_open()) {
		std::cerr << "can not open the file\n";
		in.close();
		return false;
	}
	TGAHeader header;
	in.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (!in.good()) {
		in.close();
		std::cerr << "an error occured while reading the header\n";
		return false;
	}
	w = header.width;
	h = header.height;
	bpp = header.bits_per_pixel >> 3;
	if (w <= 0 || h <= 0 || (bpp!=GRAYSCALE && bpp!= RGBA && bpp != RGB)) {
		in.close();
		std::cerr << "bad bytes_per_pixel (or width/height) value\n";
		return false;
	}
	size_t nbytes = bpp * w * h;
	data = std::vector<std::uint8_t>(nbytes, 0);
	if (3==header.image_type || 2==header.image_type) {		//  image_type == 2 || 3 : uncompressed image
		in.read(reinterpret_cast<char*>(data.data()), nbytes);
		if (!in.good()) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	}
	else if (10==header.image_type || 11==header.image_type) {	// image_type == 9 | 10: run-length encoded image
		if (!load_rle_data(in)) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	}
	else {
		in.close();
		std::cerr << "unknown file format " << (int)header.image_type << "\n";
		return false;
	}
    // Bit 4 of the image descriptor byte indicates right-to-left pixel ordering if set. 
    // Bit 5 indicates an ordering of top-to-bottom. 
    // Otherwise, pixels are stored in bottom-to-top, left-to-right order.
	if (!(header.image_descriptor & 0x20))
		flip_vertically();
	if (header.image_descriptor & 0x10)	
		flip_horizontally();
	in.close();
	return true;
}

bool TGAImage::write_tga_file(const std::string filename, const bool vflip, const bool rle) const {
	constexpr std::uint8_t developer_area_ref[4] = {0, 0, 0, 0};
	constexpr std::uint8_t extension_area_ref[4] = {0, 0, 0, 0};
	constexpr std::uint8_t footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "can not open the file " << filename << "\n";
		out.close();
		return false;
	}
	TGAHeader header;
	header.bits_per_pixel = bpp << 3;
	header.width = w;
	header.height = h;
	header.image_type = (bpp == GRAYSCALE) ? (rle ? 11 : 3) : (rle ? 10 : 2);
	header.image_descriptor = vflip ? 0x00 : 0x20; 	// top-left or bottom-left
	out.write(reinterpret_cast<const char*>(&header), sizeof(header));
	if (!out.good()) {
		std::cerr << "can not dump the tga file\n";
		out.close();
		return false;
	}
	if (!rle) {
		out.write(reinterpret_cast<const char*>(data.data()), w * h * bpp);
		if (!out.good()) {
			std::cerr << "can not unload raw data\n";
			out.close();
			return false;
		}
	}
	else if (!unload_rle_data(out)){
		std::cerr << "can not unload run-length data\n";
		out.close();
		return false;
	}
	out.write(reinterpret_cast<const char*>(developer_area_ref), sizeof(developer_area_ref));
	if (!out.good()) {
		std::cerr << "can not dump the tga file\n";
		out.close();
		return false;
	}
	out.write(reinterpret_cast<const char*>(extension_area_ref), sizeof(extension_area_ref));
	if (!out.good()) {
		std::cerr << "can not dump the tga file\n";
		out.close();
		return false;
	}
	out.write(reinterpret_cast<const char*>(footer), sizeof(footer));
	if (!out.good()) {
		std::cerr << "can not dump the tga file\n";
		out.close();
		return false;
	}
	out.close();
	return true;
}

void TGAImage::flip_horizontally() {
	int half = w >> 1;
	for (int i = 0; i < half; ++i)
		for (int j = 0; j < h; ++j)
			for (int k = 0; k < bpp; ++k)
				std::swap(data[(i+j*w)*bpp+k], data[(w-1-i+j*w)*bpp+k]);
}

void TGAImage::flip_vertically() {
	int half = h >> 1;
	for (int i = 0; i < w; ++i)
		for (int j = 0; j < half; ++j)
			for (int k = 0; k < bpp; ++k)
				std::swap(data[(i+j*w)*bpp+k], data[(i+(h-1-j)*w)*bpp + k]);
}

TGAColor TGAImage::get(const int x, const int y) const {
	if (!data.size() || x<0 || x>=w || y<0 || y>=h)
		return {};
	TGAColor ret(data.data() + (y * w + x) * bpp, bpp);
	if (bpp <= 3)
		ret.bgra[3] = 255;
	return ret;
}

void TGAImage::set(const int x, const int y, const TGAColor &c) {
	if (!data.size() || x<0 || x>=w || y<0 || y>=h)
		return;
	memcpy(data.data() + (y * w + x) * bpp, c.bgra, bpp);
}

int TGAImage::width() const {
	return w;
}

int TGAImage::height() const {
	return h;
}

void TGAImage::clear() {
	for (auto& i: data)
		i = 0;
}

bool TGAImage::load_rle_data(std::ifstream &in) {
	size_t piexl_count = w * h;
	size_t cur_pixel = 0;
	size_t cur_byte  = 0;
	TGAColor color_buf;

	do {
		std::uint8_t chunk_header;
		chunk_header = in.get();
		if (!in.good()) {
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
		if (chunk_header < 128) {	// raw packet
			chunk_header++;
			for (int i = 0; i < chunk_header; ++i) {
				in.read(reinterpret_cast<char*>(color_buf.bgra), bpp);
				if (!in.good()) {
					std::cerr << "an error occured while reading the data\n";
					return false;
				}
				for (int j = 0; j < bpp; ++j)
					data[cur_byte++] = color_buf[j];
				cur_pixel++;
				if (cur_pixel > piexl_count) {
					std::cerr << "too many pixel read\n";
					return false;
				}
			}
		}
		else {	// run-length packet
			chunk_header -= 127;
			in.read(reinterpret_cast<char*>(color_buf.bgra), bpp);
			if (!in.good()) {
				std::cerr << "an error occured while reading the data\n";
				return false;
			}
			for (int i = 0; i < chunk_header; ++i) {
				for (int j = 0; j < bpp; ++j)
					data[cur_byte++] = color_buf[j];
				cur_pixel++;
				if (cur_pixel > piexl_count) {
					std::cerr << "too many pixel read\n";
					return false;
				}
			}
		}
	} while(cur_pixel < piexl_count);
	return true;
}

bool TGAImage::unload_rle_data(std::ofstream &out) const {
	const std::uint8_t max_chunk_length = 128;
	size_t pixel_count = w * h;
	size_t cur_pixel = 0;

	while (cur_pixel < pixel_count) {
		size_t chunk_start = cur_pixel * bpp;
		size_t cur_byte = cur_pixel * bpp;
		std::uint8_t run_length = 1;
		bool is_raw = true;
		while (cur_pixel + run_length < pixel_count && run_length < max_chunk_length) {
			bool succ_eq = true;
			for (int i = 0; succ_eq && i < bpp; ++i)
				succ_eq = (data[cur_byte + i] == data[cur_byte + bpp + i]);
			cur_byte += bpp;
			if (1 == run_length)
				is_raw = !succ_eq;
			if (is_raw && succ_eq) {
				--run_length;
				break;
			}
			if (!is_raw && !succ_eq)
				break;
			++run_length;
		}
		cur_pixel += run_length;
		out.put(is_raw ? run_length - 1 : run_length + 127);
		if (!out.good()) {
			std::cerr << "can not dump the tga file\n";
			return false;
		}
		out.write(reinterpret_cast<const char*>(data.data() + chunk_start), is_raw ? run_length * bpp : bpp);
		if (!out.good()) {
			std::cerr << "can not dump the tga file\n";
			return false;
		}
	}
	return true;
}

TGAColor operator*(const TGAColor &c, float intensity) {
	TGAColor ret;
	intensity = std::clamp(intensity, 0.0f, 1.0f);
	for (int i = 0; i < 4; ++i)
		ret.bgra[i] = c.bgra[i] * intensity;
	ret.bytes_per_pixel = c.bytes_per_pixel;
	return ret;
}

TGAColor operator+(const TGAColor &lhs, const TGAColor &rhs) {
	TGAColor ret;
	ret.bytes_per_pixel = rhs.bytes_per_pixel;
	for (int i = 0; i < 4; ++i) {
		ret.bgra[i] = std::min(lhs.bgra[i] + rhs.bgra[i], 255);
	}
	return ret;
}

Color TGAColor::as_color() {
	Color ret;
	ret.r = bgra[2] / 255.0;
	ret.g = bgra[1] / 255.0;
	ret.b = bgra[0] / 255.0;
	ret.a = bgra[3] / 255.0;
	return ret;
}
