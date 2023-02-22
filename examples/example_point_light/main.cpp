#include <optional>
#include "gl.h"
#include "camera.h"
#include "buffer.h"
#include "triangle.h"
#include "model.h"
#include "texture.h"

const int width  = 800;
const int height = 800;

const vec3 light_pos(1, 1, 2);
const vec3 eye(-1, 1, 7);
const vec3 center(0, 0, 0);
const vec3 up(0, 1, 0);

class LightShader : public IShader {
public:
	Model *model;
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_proj;

	virtual vec4 vertex(int iface, int nthvert) {
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);
		return uniform_proj * uniform_view * uniform_model * gl_Vertex;
	}

	virtual std::optional<color_t> fragment(vec3 bar) {
		color_t color;
		color = color_t(1, 1, 1);
		return std::optional<color_t>(color);
	}
private:
};


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

	virtual std::optional<color_t> fragment(vec3 bar) {
		return std::optional<color_t>(color_t());
	}
private:

};

// Blinn-Phong Shading and tangent space normal map
class Shader : public IShader {
public:
	Model   *model      = nullptr;
	Texture *diff_map   = nullptr;
	Texture *shadow_map = nullptr;
	Texture *normal_map = nullptr;
	Texture *spec_map   = nullptr;
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

	virtual std::optional<color_t> fragment(vec3 bar) {
		vec2 frag_uv = varying_uv * bar;
		vec3 pos = vec3(varying_pos * bar);	// fragment position in world space

		float shadow = 0.3 + 0.7 *  visibility(bar);

		vec3 n = tbn_normal(bar);
		// vec3 l = light_dir.normalize();
		vec3 l = (light_pos - pos).normalize();
		vec3 v = (eye - pos).normalize();
		vec3 r = (v + l).normalize();

		float spec = 0;
		if (spec_map) {
			float f = spec_map->sample(frag_uv)[0] * 255;
			spec = pow(std::max(0.0, dot(r, n)), f);
		}
		float diff = std::max(0.0, dot(n, l));

		color_t c = diff_map->sample(frag_uv);
		color_t color;
		for (int i = 0; i < 3; ++i) {
			color[i] = std::min<float>(0.07 + c[i] * shadow * (1.2 * diff + 0.6 * spec), 1.0);
		}
		color[3] = c[3];

		return std::optional<color_t>(color);
	}


private:
	mat<2, 3> varying_uv;
	mat<3, 3> varying_pos;
	mat<3, 3> varying_normal;

	// tangent space to world space
	vec3 tbn_normal(vec3 bar) {
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

		color_t tmp = normal_map->sample(frag_uv);
		vec3 n = vec3(tmp.r, tmp.g, tmp.b) * 2.0 - vec3(1, 1, 1);
		n = (B * n).normalize();
		return n;
	}

	float visibility(vec3& bar) {

		vec3 p = varying_pos * bar;
		vec4 p1 = uniform_shadow * vec4(p, 1.0);
		p1 = p1 / p1.w;
		float cur_depth = p1.z;
		p1 = 0.5 * p1 + 0.5;

		float visib = 0;
		float bias = 0.05;
		vec2 texel_size = vec2(1.0/shadow_map->width(), 1.0/shadow_map->height());
		for (int x = -3; x <= 3; ++x) {
			for (int y = -3; y <= 3; ++y) {
				float pcf_depth = shadow_map->sample(vec2(p1.x, p1.y) + vec2(x, y) * texel_size)[0];
				pcf_depth = pcf_depth * 2 - 1;
				float tmp = cur_depth + bias;
				visib += tmp < pcf_depth ? 0.0 : 1.0;
			}
		}
		return visib / 49.0;
	}
};


int main(int argc, char **argv) {
	Model *sphere = new Model("../obj/sphere/sphere.obj");
	Model *head = new Model("../obj/african_head/african_head.obj");
	Model *floor  = new Model("../obj/floor/floor.obj");

	mat3 m;
	m[0] = {1, 1, 1};
	m[1] = {1, 1, 1};
	m[2] = {1, 1, 1};
	Texture head_diff("../obj/african_head/african_head_diffuse.tga");
	head_diff.convolute(1.0 / 9 * m);
	Texture head_spec("../obj/african_head/african_head_spec.tga");
	Texture head_norm("../obj/african_head/african_head_nm_tangent.tga");
	Texture floor_diff("../obj/floor/floor_diffuse.tga");
	Texture floor_norm("../obj/floor/floor_nm_tangent.tga");
	floor_diff.convolute(1.0 / 9 * m);

	Camera camera(eye, center, up);
	mat4 vp = viewport(0, 0, width, height);
	mat4 view = camera.get_view_mat();
	mat4 proj = perspective(radius(45), (float)width / (float)height, -0.1f, -30.0f);
	// scale before translate
	mat4 sphere_model = scale(mat4::identity(), vec3(0.03, 0.03, 0.03));
	sphere_model = translate(sphere_model, light_pos);
	mat4 head_model = translate(mat4::identity(), vec3(0, -1.0, 0));
	mat4 floor_model = scale(mat4::identity(), vec3(2, 2, 2));
	floor_model = translate(floor_model, vec3(-1, 0, -1));

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage shadow_map(width, height, TGAImage::GRAYSCALE);
	DepthBuffer shadow_buf(width, height, -std::numeric_limits<float>::max(), 4);
	DepthBuffer zbuf(width, height, -std::numeric_limits<float>::max(), 4);
	ColorBuffer color_buf(width, height, color_t(0, 0, 0, 0), 4);


	// gen shadow map
	Camera tmp(light_pos, center, up);
	// mat4 shadow_view = tmp.get_view_mat();
	mat4 shadow_view = lookat(light_pos, center, up);
	mat4 shadow_proj = perspective(radius(120), (float)width/(float)height, -1.1f, -100.0f);
	{
		DepthShader d_shader;
		d_shader.uniform_projection = shadow_proj;
		d_shader.uniform_view = shadow_view;

		d_shader.uniform_model = head_model;
		d_shader.model = head;
		head->draw(d_shader, vp, shadow_buf, nullptr, Triangle::MSAA4);

		d_shader.uniform_model = floor_model;
		d_shader.model = floor;
		floor->draw(d_shader, vp, shadow_buf, nullptr, Triangle::MSAA4);

		for (int x = 0; x < width; ++x) {
			for (int y = 0; y < height; ++y) {
				// uint8_t depth = 127.5 * shadow_buf.get_value(x, y) + 127.5;
				depth_t depth = shadow_buf.get_value(x, y);
				depth = depth / 2 + 0.5;
				color_t c(depth);
				shadow_map.set(x, y, c);
			}
		}
		shadow_map.write_tga_file("point_light_shadow.tga");
	}

	// render image
	{
		// draw light source
		LightShader l_shader;
		l_shader.uniform_view = view;
		l_shader.uniform_proj = proj;
		l_shader.uniform_model = sphere_model;
		l_shader.model = sphere;
		sphere->draw(l_shader, vp, zbuf, &color_buf, Triangle::MSAA4);

		Shader shader;
		shader.uniform_projection = proj;
		shader.uniform_view = view;
		shader.uniform_shadow = shadow_proj * shadow_view;
		Texture s_map(std::move(shadow_map));
		shader.shadow_map = &s_map;
		// shader.uniform_shadow_map = &shadow_map;

		// draw head
		shader.uniform_model = head_model;
		shader.diff_map = &head_diff;
		shader.normal_map = &head_norm;
		shader.spec_map = &head_spec;
		shader.model = head;
		head->draw(shader, vp, zbuf, &color_buf, Triangle::MSAA4);

		// draw floor
		shader.uniform_model = floor_model;
		shader.diff_map = &floor_diff;
		shader.spec_map = nullptr;
		shader.normal_map = &floor_norm;
		shader.model = floor;
		floor->draw(shader, vp, zbuf, &color_buf, Triangle::MSAA4);

		for (int x = 0; x < width; ++x) {
			for (int y = 0; y < height; ++y) {
				image.set(x, y, color_t(color_buf.get_value(x, y)));
			}
		}
	}


	image.write_tga_file("point_light.tga");
	return 0;
}
