#include "gl.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(1, 1, 3);
// vec3 eye(0, 0, 2);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

mat4 View;
mat4 Projection;
mat4 Viewport;


class TShader : public IShader {
public:
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_projection;
	mat4 uniform_viewport;
	mat4 uniform_MIT;

	virtual vec4 vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		vec4 gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return uniform_viewport * uniform_projection * uniform_view * uniform_model * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		vec2 frag_uv = varying_uv * bar;
		// vec3 n = proj<3>(uniform_MIT * embed<4>(model->normal(frag_uv), 0.0)).normalize();
		vec3 n = vec3(uniform_MIT * vec4(model->normal(frag_uv), 0.0)).normalize();
		vec3 l = light_dir.normalize();

		float diff = std::max(0.0, n * l);
		// color = model->diffuse(frag_uv) * diff;
		color = model->diffuse(frag_uv) *diff ;
		
		return false;
	}

private:
	mat<2, 3> varying_uv;
};

class Shader : public IShader {
public:
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_projection;
	mat4 uniform_viewport;
	mat4 uniform_MIT;
	virtual vec4 vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		vec4 gl_Vertex = embed<4>(model->vert(iface, nthvert));
		varying_frag_pos.set_col(nthvert, uniform_model * gl_Vertex);
		return uniform_viewport * uniform_projection * uniform_view * uniform_MIT * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		vec2 frag_uv = varying_uv * bar;
		vec3 frag_pos = vec3(varying_frag_pos * bar);
		// vec3 n = proj<3>(uniform_MIT * embed<4>(model->normal(frag_uv))).normalize();
		vec3 n = vec3(uniform_MIT * vec4(model->normal(frag_uv), 0.0)).normalize();
		vec3 l = light_dir.normalize();
		vec3 r = reflect(n, -1.0 * l);
		vec3 v = (eye - frag_pos).normalize();

		float spec = pow(std::max(0.0, r * v), model->specular(frag_uv));
		float diff = std::max(0.0, n * l);
		float tmp = model->specular(frag_uv);

		TGAColor c = model->diffuse(frag_uv);
		for (int i = 0; i < 3; ++i)
			color[i] = std::min(5 + c[i] * (diff + 0.6 * spec), 255.0);

		return false;
	}

private:
	mat<2, 3> varying_uv;
	mat<4, 3> varying_frag_pos;
};


int main(int argc, char **argv) {
	model = new Model("../obj/african_head/african_head.obj");

	TGAImage image(width, height, TGAImage::RGB);
	float *zbuf = new float[width * height];
	for (int i = 0; i < width * height; ++i)
		zbuf[i] = -std::numeric_limits<float>::max();

	light_dir.normalize();
	View = lookat(eye, center, up);
	Projection = perspective(radius(45), (float)width / (float)height, -0.1, -100.0);
	Viewport = viewport(0, 0, width, height);

	Shader shader;
	shader.uniform_model = mat4::identity();
	shader.uniform_view = lookat(eye, center, up);
	shader.uniform_projection = perspective(radius(45), (float)width / (float)height, -0.1, -100.0); 
	shader.uniform_viewport = viewport(0, 0, width, height);
	shader.uniform_MIT = (mat4::identity()).invert_transpose();

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

	delete model;
	return 0;
}
