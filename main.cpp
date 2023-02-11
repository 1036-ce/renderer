#include <optional>
#include "gl.h"
#include "camera.h"
#include "buffer.h"
#include "triangle.h"
#include "model.h"

const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(1, 1, 7);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

class DepthShader : public IShader {
public:
	Model *model = nullptr;
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_projection;

	virtual vec4 vertex(int iface, int nthvert) {
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);
		return uniform_projection * uniform_view * uniform_model * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		return false;
	}
private:

};

// Blinn-Phong Shading and tangent space normal map
class Shader : public IShader {
public:
	Model *model = nullptr;
	TGAImage *shadow_map;
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_projection;
	mat4 uniform_vp;
	mat4 uniform_shadow;

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

		float shadow = 0.3 + 0.7 * !is_shadow(bar);

		vec3 n = calc_normal(bar);
		vec3 l = light_dir.normalize();
		vec3 v = (eye - pos).normalize();
		vec3 r = (v + l).normalize();

		float spec = pow(std::max(0.0, dot(r, n)), model->specular(frag_uv));
		float diff = std::max(0.0, dot(n, l));

		TGAColor c = model->diffuse(frag_uv);
		for (int i = 0; i < 3; ++i) {
			color[i] = std::min<float>(20 + c[i] * shadow * (1.2 * diff + .6 * spec), 255);
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

	float is_shadow(vec3& bar) {

		vec3 p = varying_pos * bar;
		vec4 p1 = uniform_shadow * vec4(p, 1.0);
		p1 = p1 / p1.w;
		float cur_depth = p1.z;
		p1 = 0.5 * p1 + 0.5;
		float closest_depth = texture(shadow_map, vec2(p1.x, p1.y))[0];

		// shadow bias 
		float bias = 0.05;
		cur_depth += bias;
		cur_depth = 127.5 * cur_depth + 127.5;
		return cur_depth < closest_depth ? 1.0 : 0.0;
	}
};

int main(int argc, char **argv) {
	// Model *model = nullptr;
	Model *head = new Model("../obj/african_head/african_head.obj");
	Model *floor  = new Model("../obj/floor/floor.obj");
	mat4 vp = viewport(0, 0, width, height);

	mat4 floor_model = mat4::identity();
	floor_model = scale(floor_model, vec3(2, 2, 2));
	floor_model = translate(floor_model, vec3(-1, 0, -1));
	mat4 head_model = mat4::identity();
	head_model = translate(head_model, vec3(0, -1.0, 0));

	DepthBuffer depth_buf(width, height, -std::numeric_limits<float>::max(), 4);
	TGAImage depth_map(width, height, TGAImage::GRAYSCALE);
	DepthShader d_shader;
	mat4 light_model = mat4::identity();
	mat4 light_view  = lookat(vec3(1, 1, 1), center, up);
	mat4 light_proj  = orthographic(-4.0, 4.0, -0.1, -10.0, -4.0, 4.0);
	d_shader.uniform_view  = light_view;
	d_shader.uniform_projection = light_proj;


	// generate shadow map
	{
		d_shader.uniform_model = floor_model;
		d_shader.model = floor;
		floor->draw(d_shader, vp, depth_buf, nullptr, Triangle::MSAA4);

		d_shader.uniform_model = head_model;
		d_shader.model = head;
		head->draw(d_shader, vp, depth_buf, nullptr, Triangle::MSAA4);

		for (int i = 0; i < width; ++i) {
			for (int j = 0; j < height; ++j) {
				uint8_t depth = 127.5 * depth_buf.get_value(i, j) + 127.5;
				TGAColor c(depth);
				depth_map.set(i, j, c);
			}
		}
		depth_map.write_tga_file("depth_map.tga");
		system("convert depth_map.tga depth_map.png");
		system("mv depth_map.png ../");
	}



	Camera camera(eye, center, up);
	TGAImage image(width, height, TGAImage::RGB);

	Shader shader;
	mat4 model_mat = mat4::identity();
	mat4 model_view = camera.get_view_mat();
	mat4 model_proj = perspective(radius(45), (float)width / (float)height, -0.1, -100.0);
	shader.uniform_model = model_mat;
	shader.uniform_view = model_view;
	shader.uniform_projection = model_proj;
	shader.uniform_vp = vp;
	shader.shadow_map = &depth_map;
	shader.uniform_shadow = light_proj * light_view;

	ColorBuffer color_buf = ColorBuffer(width, height, TGAColor(0, 0, 0), 4);
	DepthBuffer zbuf(width, height, -std::numeric_limits<float>::max(), 4);

	// generate image
	{
		shader.uniform_model = floor_model;
		shader.model = floor;
		floor->draw(shader, vp, zbuf, &color_buf, Triangle::MSAA4);

		shader.uniform_model = head_model;
		shader.model = head;
		head->draw(shader, vp, zbuf, &color_buf, Triangle::MSAA4);

		for (int x = 0; x < width; ++x) {
			for (int y = 0; y < height; ++y) {
				TGAColor c = color_buf.get_value(x, y);
				image.set(x, y, c);
			}
		}

		image.write_tga_file("output.tga");
		system("convert output.tga output.png");
		system("mv output.png ../");
	}

	delete head;
	delete floor;
	return 0;
}
