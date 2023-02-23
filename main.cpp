#include <optional>
#include "gl.h"
#include "camera.h"
#include "buffer.h"
#include "triangle.h"
#include "model.h"
#include "texture.h"

const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(1, 1, 7);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

class GouraudShader : public IShader {
public:
	Model *model;
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_proj;

	virtual vec4 vertex(int iface, int nthvert) {
		vec3 n = model->normal(iface, nthvert).normalize();
		varying_intensity[nthvert] = std::max(0.0, dot(n, light_dir)); // get diffuse lighting intensity
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);		// read the vertex from .obj file
		return uniform_proj * uniform_view * uniform_model * gl_Vertex;		// transform it to screen coordinates
	}

	virtual bool fragment(vec3 bar, TGAColor &color) {
		float intensity = dot(varying_intensity, bar);	// interpolate intensity for the current pixel
		color = TGAColor(255, 255, 255) * intensity;
		return false;		// don't discard the pixel;
	}
private:
	vec3 varying_intensity;		// writen by vertex shader, read by fragment shader
};


class Shader : public IShader {
public:
	Model *model;
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_proj;
	virtual vec4 vertex(int iface, int nthvert) {
		vec3 n = model->normal(iface, nthvert).normalize();
		light_dir = light_dir.normalize();
		varying_intensity[nthvert] = std::max(0.0, dot(n, light_dir)); // get diffuse lighting intensity
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);		// read the vertex from .obj file
		return uniform_proj * uniform_view * uniform_model * gl_Vertex;		// transform it to screen coordinates
	}

	virtual std::optional<color_t> fragment(vec3 bar) {
		float intensity = dot(varying_intensity, bar);	// interpolate intensity for the current pixel
		color_t color = color_t(1, 1, 1) * intensity;
		return std::optional<color_t>(color);
	}
private:
	vec3 varying_intensity;		// writen by vertex shader, read by fragment shader
};

int main() {
	Model *cube = new Model("../obj/cube/cube.obj");
	Camera camera(eye, center, up);
	mat4 vp = viewport(0, 0, width, height);

	mat4 cube_model = mat4::identity();
	cube_model = rotate(cube_model, 20, vec3(1, 1, 1));
	mat4 cube_view  = camera.get_view_mat();
	mat4 cube_proj  = perspective(radius(45), (float)width/height, -0.1f, -100.0f);

	Shader shader;
	shader.model = cube;
	shader.uniform_model = cube_model;
	shader.uniform_view = cube_view;
	shader.uniform_proj = cube_proj;

	DepthBuffer zbuf(width, height, -std::numeric_limits<float>::max(), 4);
	ColorBuffer color_buf(width, height, color_t(0, 0, 0, 0), 4);

	cube->draw(shader, vp, zbuf, &color_buf, Triangle::MSAA4);

	TGAImage image(width, height, TGAImage::RGB);
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < width; ++y) {
			image.set(x, y, color_buf.get_value(x, y));
		}
	}

	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png ../");

	return 0;
}