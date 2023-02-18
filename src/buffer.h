#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include "tgaimage.h"

template<typename T> class Buffer {
public:
	Buffer() = default;
	Buffer(int width, int height, T fill, int simples = 1)
		: w(width), h(height), simples(simples), 
		data(width * height, std::vector<T>(simples, fill)) {}
	T 	 get(int x, int y, int nthsimple);
	void set(int x, int y, int nthsimple, T t);
	// return average value in (x, y)
	T    get_value(int x, int y);
	int  simple_num() { return simples; }
	int  width()  { return w; }
	int  height() { return h; }
private:
	int w;
	int h;
	int simples;
	std::vector<std::vector<T>> data;
};

template <typename T> 
inline T Buffer<T>::get(int x, int y, int nthsimple) {
	assert(nthsimple >= 0 && nthsimple < simples);
	return data[y * w + x][nthsimple];
}

template <typename T>
inline void Buffer<T>::set(int x, int y, int nthsimple, T t) {
	assert(nthsimple >= 0 && nthsimple < simples);
	data[y * w + x][nthsimple] = t;
}

template <typename T> inline T Buffer<T>::get_value(int x, int y) {
	assert(x >= 0 && x < w);
	assert(y >= 0 && y < h);
	std::vector<T>& v = data[y * w + x];
	T ret = v[0] * (1.0 / simples);
	for (int i = 1; i < v.size(); ++i)
		ret = ret + v[i] * (1.0 / simples);
	// for (T &t: v) {
	// 	ret = ret + t * (1.0 / simples);
	// }
	return ret;
}

using DepthBuffer = Buffer<float>;
using ColorBuffer = Buffer<TGAColor>;