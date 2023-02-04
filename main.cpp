#include "gl.h"
#include "camera.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 0);
vec3 eye(1, 1, 4);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

int main(int argc, char **argv) {
	if (2 == argc)
		model = new Model(argv[1]);
	else {
		model = new Model("../obj/diablo3_pose/diablo3_pose.obj");
		// model = new Model("../obj/african_head/african_head.obj");
	}

	Camera camera(eye, center, up);
	TGAImage image(width, height, TGAImage::RGB);
	float *zbuf = new float[width * height];
	for (int i = 0; i < width * height; ++i) {
		zbuf[i] 	  = -std::numeric_limits<float>::max();
	}


	Shader shader;
	shader.uniform_model = model_mat;
	shader.uniform_view = camera.get_view_mat();
	shader.uniform_projection = perspective(radius(45), (float)width / (float)height, -0.1, -100.0); 
	shader.uniform_viewport = viewport(0, 0, width, height);
	shader.uniform_MIT = model_mat.invert_transpose();
	shader.uniform_sbuf = shadow_buf;
	shader.uniform_shadow = vp * proj * view;

	// draw model
	for (int i = 0; i < model->nfaces(); ++i) {
		vec4 screen_coord[3];
		for (int j = 0; j < 3; ++j) {
			screen_coord[j] = shader.vertex(i, j);
		}
		triangle(screen_coord, shader, image, zbuf);
	}
	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png ../");

	TGAImage shadow_image(width, height, TGAImage::GRAYSCALE);
	get_zbuf_image(shadow_buf, width, height, shadow_image);
	shadow_image.write_tga_file("shadow_buf.tga");
	system("convert shadow_buf.tga shadow_buf.png");
	system("mv shadow_buf.png ../");

	delete model;
	return 0;
}
