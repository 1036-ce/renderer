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


class TShader : public IShader {
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

		color = model->diffuse(frag_uv);
		// color = TGAColor(123, 231, 12);

		return false;
	}
private:
	mat<2, 3> varying_uv;
};


// Blinn-Phong Shading and tangent space normal map
class Shader : public IShader {
public:
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_projection;
	mat4 uniform_MIT;

	virtual vec4 vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_normal.set_col(nthvert, model->normal(iface, nthvert));
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);
		gl_Vertex = uniform_model * gl_Vertex;
		varying_pos.set_col(nthvert, vec3(gl_Vertex));
		return uniform_projection * uniform_view * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		vec2 frag_uv = varying_uv * bar;
		vec3 pos = vec3(varying_pos * bar);	// fragment position in world space

		vec3 n = calc_normal(bar);
		vec3 l = light_dir.normalize();
		vec3 v = (eye - pos).normalize();
		vec3 r = (v + l).normalize();

		float spec = pow(std::max(0.0, dot(r, n)), model->specular(frag_uv));
		float diff = std::max(0.0, dot(n, l));

		TGAColor c = model->diffuse(frag_uv);
		for (int i = 0; i < 3; ++i) {
			color[i] = std::min(5 + c[i] * (diff + 0.6 * spec), 255.0);
		}
		color[3] = c[3];
		color.bytes_per_pixel = c.bytes_per_pixel;

		return false;
	}


private:
	mat<2, 3> varying_uv;
	mat<3, 3> varying_pos;
	mat<3, 3> varying_normal;

	// tangent space to world space
	vec3 calc_normal(vec3 bar) {
		vec2 frag_uv = varying_uv * bar;
		vec3 bn = (varying_normal * bar).normalize();
		mat3 A;
		A[0] = varying_pos.col(1) - varying_pos.col(0);
		A[1] = varying_pos.col(2) - varying_pos.col(0);
		A[2] = bn;
		A = A.invert();

		vec3 i = A * vec3(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		vec3 j = A * vec3(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);
		mat3 B;
		B.set_col(0, i.normalize());
		B.set_col(1, j.normalize());
		B.set_col(2, bn);

		vec3 n = (B * model->normal(frag_uv)).normalize();
		return n;
	}
};

int main(int argc, char **argv) {
	if (2 == argc)
		model = new Model(argv[1]);
	else {
		model = new Model("../obj/african_head/african_head.obj");
		// model = new Model("../obj/diablo3_pose/diablo3_pose.obj");
	}

	Camera camera(eye, center, up);
	TGAImage image(width, height, TGAImage::RGB);
	ColorBuffer color_buf(width, height, TGAColor(0, 0, 0), 4);
	DepthBuffer depth_buf(width, height, -std::numeric_limits<float>::max(), 4);

	Shader shader;
	mat4 model_mat = mat4::identity();
	shader.uniform_model = model_mat;
	shader.uniform_view = camera.get_view_mat();
	shader.uniform_projection = perspective(radius(45), (float)width / (float)height, -0.1, -100.0); 
	shader.uniform_MIT = model_mat.invert_transpose();

	mat4 vp = viewport(0, 0, width, height);

	model->draw(shader, vp, depth_buf, color_buf, Triangle::MSAA4);

	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			TGAColor c = color_buf.get_value(x, y);
			image.set(x, y, c);
		}
	}

	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png ../");

	delete model;
	return 0;
}
