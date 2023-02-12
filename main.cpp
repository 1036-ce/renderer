#include <optional>
#include "gl.h"
#include "camera.h"
#include "buffer.h"
#include "triangle.h"
#include "model.h"

const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(1, 1, 9);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

class Shader: public IShader {
public:
	Model *model;
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_proj;

	virtual vec4 vertex(int iface, int nthvert) {
		vec3 gl_Vertex = model->vert(iface, nthvert);
		return uniform_proj * uniform_view * uniform_model * vec4(gl_Vertex, 1.0);
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		color = TGAColor(122, 23, 45);
		return false;
	}
private:
};

int main(int argc, char **argv) {
	Model *rabbit = new Model("../obj/teapot/teapot.obj");
	// Model *rabbit = new Model("../obj/rabbit/rabbit.obj");
	mat4 vp = viewport(0, 0, width, height);
	Camera camera(eye, center, up);
	DepthBuffer zbuf(width, height, -std::numeric_limits<float>::max(), 4);
	ColorBuffer color_buf(width, height, TGAColor(0, 0, 0), 4);

	mat4 rabbit_model = mat4::identity();
	// rabbit_model = scale(rabbit_model, vec3(12, 12, 12));
	rabbit_model = translate(rabbit_model, vec3(0, -1, 0));
	mat4 rabbit_view  = camera.get_view_mat();
	mat4 rabbit_proj  = perspective(radius(45), (float)width / (float)height, -0.1, -100.0);

	Shader shader;
	shader.model = rabbit;
	shader.uniform_model = rabbit_model;
	shader.uniform_view = rabbit_view;
	shader.uniform_proj = rabbit_proj;

	rabbit->draw(shader, vp, zbuf, &color_buf, Triangle::MSAA4);

	TGAImage image(width, height, TGAImage::RGB);
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			image.set(x, y, color_buf.get_value(x, y));
		}
	}

	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png ../");

	return 0;
}
