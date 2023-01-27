#include "gl.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(2, 0, 3);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

mat4 ModelView;
mat4 Projection;
mat4 Viewport;

class Shader : public IShader {
public:
	virtual vec4 vertex(int iface, int nthvert) {
		varying_intensity[nthvert] = std::max(0.0, model->normal(iface, nthvert).normalize() * light_dir);
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		vec4 gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		float intensity = varying_intensity * bar;
		vec2 frag_uv = varying_uv * bar;
		color = model->diffuse(frag_uv);
		return false;
	}
private:
	vec3 varying_intensity;
	mat<2, 3> varying_uv;
};

int main() {
	model = new Model("../obj/african_head/african_head.obj");

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuf(width, height, TGAImage::GRAYSCALE);
	// PhongShader shader;
	Shader shader;

	light_dir.normalize();
	ModelView = lookat(eye, center, up);
	std::cout << "ModelView:" << std::endl;
	std::cout << ModelView << std::endl;

	Projection = projection((eye - center).norm());
	std::cout << "Projection:" << std::endl;
	std::cout << Projection << std::endl;

	Viewport = viewport(width/8, height/8, width*3/4, height*3/4);
	std::cout << "Viewport:" << std::endl;
	std::cout << Viewport << std::endl;

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
	return 0;
}
