#include "gl.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(1, 1, 3);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

mat4 ModelView;
mat4 Projection;
mat4 Viewport;


class Shader : public IShader {
public:
	virtual vec4 vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		vec4 gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		vec2 frag_uv = varying_uv * bar;
		vec3 n = proj<3>(uniform_MIT * embed<4>(model->normal(frag_uv))).normalize();
		vec3 l = proj<3>(uniform_M   * embed<4>(			 light_dir)).normalize();
		vec3 r = reflect(n, l);

		float spec = pow(std::max(0.0, r.z), model->specular(frag_uv));
		float diff = std::max(0.0, n * l);
		float tmp = model->specular(frag_uv);

		TGAColor c = model->diffuse(frag_uv);
		for (int i = 0; i < 3; ++i)
			color[i] = std::min(5 + c[i] * (diff + 0.6 * spec), 255.0);

		return false;
	}

	mat4 uniform_M;		// Projection * ModelView;
	mat4 uniform_MIT;	// (Projection * ModelView).invert_transpose()
private:
	mat<2, 3> varying_uv;
};



int main() {
	model = new Model("../obj/african_head/african_head.obj");

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuf(width, height, TGAImage::GRAYSCALE);

	light_dir.normalize();
	ModelView = lookat(eye, center, up);
	Projection = projection((eye - center).norm());
	Viewport = viewport(width/8, height/8, width*3/4, height*3/4);

	Shader shader;
	// GouraudShader shader;
	shader.uniform_MIT = (Projection * ModelView).invert_transpose();
	shader.uniform_M   = Projection * ModelView;

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
	zbuf.write_tga_file("zbuf.tga");
	system("convert zbuf.tga zbuf.png");
	system("mv zbuf.png ../");

	delete model;

}
