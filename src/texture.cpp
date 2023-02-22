#include "texture.h"

color_t Texture::sample(const vec2 &uv) {
	int x = image.width() * uv.x;
	int y = image.height() * uv.y;
	return image.get(x, y);
}