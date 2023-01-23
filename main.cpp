#include "gl.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0  , 255, 0,   255);

void draw() {
	Model *model = new Model("/home/zhaosiqi/workspace/Projects/renderer/obj/african_head/african_head.obj");

	int width = 2000, height = 2000;
	vec3 light_dir(0, 0, -1);
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage diffuse = model->diffuse();
	diffuse.flip_vertically();
	float *zbuf = new float[width * height];
	for (int i = width * height; i--; zbuf[i] = -std::numeric_limits<float>::max());

	mat4 m = mat4::identity();
	m[3][2] = -1.0 / 5.0;		// the camera in (0, 0, 5.0);

	for (int i = 0; i < model->nfaces(); ++i) {
		vec3 screen_coords[3];
		vec3 world_coords[3];
		for (int j = 0; j < 3; ++j) {
			vec3 v = model->vert(i, j);
			world_coords[j] = v;

			vec4 v1 = embed<4, 3>(v, 1.0);
			v1 = m * v1;
			for (int i = 3; i--; v[i] = v1[i] / v1[3]);
			screen_coords[j].x = width  * (v.x + 1.0) / 2.0;
			screen_coords[j].y = height * (v.y + 1.0) / 2.0;
			screen_coords[j].z = v.z;
		}
		vec3 n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]).normalize();
		float intensity = n * light_dir;
		vec2 uv[3];
		for (int j = 0; j < 3; ++j)
			uv[j] = model->uv(i, j);
		if (intensity > 0)
			triangle(screen_coords, zbuf, image, uv, diffuse, intensity);
		// 	triangle(screen_coords, zbuf, image, TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255));
	}
	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png /home/zhaosiqi/workspace/Projects/renderer");
}

int main() {
	draw();
	return 0;
}
