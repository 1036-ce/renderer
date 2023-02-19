#include "gl.h"
#include "camera.h"
#include "buffer.h"
#include "triangle.h"
#include "model.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(1, 1, 3);
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

	virtual std::optional<TGAColor> fragment(vec3 bar) {
		vec2 frag_uv = varying_uv * bar;

		TGAColor color;
		color = model->diffuse(frag_uv);
		// color = TGAColor(123, 231, 12);

		return std::optional<TGAColor>(color);
	}
private:
	mat<2, 3> varying_uv;
};

int main(int argc, char **argv) {
	if (2 == argc)
		model = new Model(argv[1]);
	else {
		model = new Model("../obj/floor/floor.obj");
	}

	Camera camera(eye, center, up);
	TGAImage image(width, height, TGAImage::RGB);

	Shader shader;
	mat4 model_mat = mat4::identity();
	shader.uniform_model = model_mat;
	shader.uniform_view = camera.get_view_mat();
	shader.uniform_projection = perspective(radius(45), (float)width / (float)height, -0.1, -100.0); 
	shader.uniform_MIT = model_mat.invert_transpose();

	mat4 vp = viewport(0, 0, width, height);

	ColorBuffer color_buf(width, height, TGAColor(0, 0, 0), 4);
	DepthBuffer depth_buf(width, height, -std::numeric_limits<float>::max(), 4);
	// draw model
	model->draw(shader, vp, depth_buf, &color_buf, Triangle::MSAA4);
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			TGAColor c = color_buf.get_value(x, y);
			image.set(x, y, c);
		}
	}

	image.write_tga_file("example_msaa.tga");
	delete model;
	return 0;
}
