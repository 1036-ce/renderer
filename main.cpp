#include "gl.h"
#include "camera.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(1, 1, 4);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);


class Shader : public IShader {
public:
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_projection;
	mat4 uniform_MIT;

	virtual vec4 vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);
		gl_Vertex = uniform_model * gl_Vertex;
		gl_Vertex = uniform_projection * uniform_view * gl_Vertex;
		return gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		vec2 frag_uv = varying_uv * bar;

		TGAColor c = model->diffuse(frag_uv);
		for (int i = 0; i < 3; ++i) {
			color[i] = c[i];
		}

		return false;
	}


private:
	mat<2, 3> varying_uv;
};


int main(int argc, char **argv) {
	if (2 == argc)
		model = new Model(argv[1]);
	else {
		// model = new Model("../obj/floor/floor.obj");
		model = new Model("../obj/african_head/african_head.obj");
	}

	Camera camera(eye, center, up);
	TGAImage image(width, height, TGAImage::RGB);
	float *zbuf = new float[width * height];
	for (int i = 0; i < width * height; ++i) {
		zbuf[i] 	  = -std::numeric_limits<float>::max();
	}


	Shader shader;
	mat4 model_mat = mat4::identity();
	shader.uniform_model = model_mat;
	shader.uniform_view = camera.get_view_mat();
	shader.uniform_projection = perspective(radius(45), (float)width / (float)height, -0.1, -100.0); 
	shader.uniform_MIT = model_mat.invert_transpose();

	// draw model
	for (int i = 0; i < model->nfaces(); ++i) {
		vec4 clip_coord[3];
		for (int j = 0; j < 3; ++j) {
			clip_coord[j] = shader.vertex(i, j);
		}
		triangle(clip_coord, shader, viewport(0, 0, width, height), zbuf, image);
		// triangle(clip_coord, shader, image, zbuf);
	}
	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png ../");

	delete model;

	TGAImage zimage(width, height, TGAImage::GRAYSCALE);
	get_zbuf_image(zbuf, zimage);
	zimage.write_tga_file("zimage.tga");
	system("convert zimage.tga zimage.png");
	system("mv zimage.png ../");

	return 0;
}
