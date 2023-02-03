#include "gl.h"
#include "camera.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 0);
vec3 eye(1, 1, 4);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

class DepthShader: public IShader {
public:
	mat4 uniform_mat;

	virtual vec4 vertex(int iface, int nthvert) {
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);
		return uniform_mat * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		color = TGAColor(255, 255, 255);
		return false;
	}
};

class Shader : public IShader {
public:
	mat4 uniform_model;
	mat4 uniform_view;
	mat4 uniform_projection;
	mat4 uniform_viewport;
	mat4 uniform_MIT;
	mat4 uniform_shadow;
	float *uniform_sbuf;

	virtual vec4 vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_normal.set_col(nthvert, model->normal(iface, nthvert));
		vec4 gl_Vertex = vec4(model->vert(iface, nthvert), 1.0);
		gl_Vertex = uniform_model * gl_Vertex;
		varying_frag_pos.set_col(nthvert, gl_Vertex);
		varying_tri.set_col(nthvert, vec3(gl_Vertex / gl_Vertex[3]));
		return uniform_viewport * uniform_projection * uniform_view * gl_Vertex;
	}

	virtual bool fragment(vec3 bar, TGAColor& color) {
		vec2 frag_uv = varying_uv * bar;
		vec3 frag_pos = vec3(varying_frag_pos * bar);

		float shadow = 0.3 + 0.7 * is_shadow(bar);

		vec3 n = calc_normal(bar);
		vec3 l = light_dir.normalize();
		vec3 v = (eye - frag_pos).normalize();
		vec3 r = (v + l).normalize();

		float spec = pow(std::max(0.0, r * n),  model->specular(frag_uv));
		// float spec = pow(std::max(0.0, r * n), 1);
		float diff = std::max(0.0, n * l);
		float tmp = model->specular(frag_uv);

		TGAColor c = model->diffuse(frag_uv);
		for (int i = 0; i < 3; ++i) {
			color[i] = std::min<float>(20 + c[i] * shadow * (1.2 * diff + .6 * spec), 255);
		}

		return false;
	}


private:
	mat<2, 3> varying_uv;
	mat<4, 3> varying_frag_pos;
	mat<3, 3> varying_normal;
	mat<3, 3> varying_tri;

	// tangent space to world space
	vec3 calc_normal(vec3 bar) {
		vec2 frag_uv = varying_uv * bar;
		vec3 bn = (varying_normal * bar).normalize();
		mat3 A;
		A[0] = varying_tri.col(1) - varying_tri.col(0);
		A[1] = varying_tri.col(2) - varying_tri.col(0);
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

	bool is_shadow(vec3 bar) {
		vec3 p = varying_tri * bar;
		vec4 p1 = uniform_shadow * vec4(p, 1.0);
		p1 = p1 / p1.w;
		int x = (int)(p1.x + 0.5);
		int y = (int)(p1.y + 0.5);
		float depth = p1.z;
		return depth < uniform_sbuf[y * width + x];
	}
};


int main(int argc, char **argv) {
	if (2 == argc)
		model = new Model(argv[1]);
	else {
		model = new Model("../obj/diablo3_pose/diablo3_pose.obj");
		// model = new Model("../obj/african_head/african_head.obj");
	}

	Camera camera(eye, center, up);
	TGAImage image(width, height, TGAImage::RGB);
	float *shadow_buf = new float[width * height];
	float *zbuf = new float[width * height];
	for (int i = 0; i < width * height; ++i) {
		shadow_buf[i] = -std::numeric_limits<float>::max();
		zbuf[i] 	  = -std::numeric_limits<float>::max();
	}

	DepthShader d_shader;
	mat4 model_mat = mat4::identity();
	mat4 view = lookat(vec3(1.5, 1.5, 1.5), vec3(0, 0, 0), vec3(0, 0, 1));
	mat4 proj = orthographic(-1, 1, -0.1, -100, -1, 1);
	mat4 vp   = viewport(0, 0, width, height);
	d_shader.uniform_mat = vp * proj * view * model_mat;

	// generate shadow_buf
	for (int i = 0; i < model->nfaces(); ++i) {
		vec4 screen_coord[3];
		for (int j = 0; j < 3; ++j) {
			screen_coord[j] = d_shader.vertex(i, j);
		}
		triangle(screen_coord, d_shader, image, shadow_buf);
	}
	image.clear();

	Shader shader;
	shader.uniform_model = model_mat;
	shader.uniform_view = camera.get_view_mat();
	shader.uniform_projection = perspective(radius(45), (float)width / (float)height, -0.1, -100.0); 
	shader.uniform_viewport = viewport(0, 0, width, height);
	shader.uniform_MIT = model_mat.invert_transpose();
	shader.uniform_sbuf = shadow_buf;
	shader.uniform_shadow = vp * proj * view;

	// draw model
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

	TGAImage shadow_image(width, height, TGAImage::GRAYSCALE);
	get_zbuf_image(shadow_buf, width, height, shadow_image);
	shadow_image.write_tga_file("shadow_buf.tga");
	system("convert shadow_buf.tga shadow_buf.png");
	system("mv shadow_buf.png ../");

	delete model;
	return 0;
}
