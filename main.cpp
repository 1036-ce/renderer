#include "gl.h"

Model *model 	 = nullptr;
const int width  = 800;
const int height = 800;

vec3 light_dir(1, 1, 1);
vec3 eye(0, -1, 3);
vec3 center(0, 0, 0);
vec3 up(0, 1, 0);

mat4 ModelView;
mat4 Projection;
mat4 Viewport;

class GouraudShader : public IShader {
public:
	vec3 varying_intensity;		// writen by vertex shader, read by fragment shader

	virtual vec4 vertex(int iface, int nthvert) {
		vec3 n = model->normal(iface, nthvert).normalize();
		varying_intensity[nthvert] = std::max(0.0, n * light_dir); // get diffuse lighting intensity
		vec4 gl_Vertex = embed<4>(model->vert(iface, nthvert));		// read the vertex from .obj file
		return Viewport * Projection * ModelView * gl_Vertex;		// transform it to screen coordinates
	}

	virtual bool fragment(vec3 bar, TGAColor &color) {
		float intensity = varying_intensity * bar;	// interpolate intensity for the current pixel
		color = TGAColor(255, 255, 255) * intensity;
		return false;		// don't discard the pixel;
	}
};

void draw() {
	Model *model = new Model("../obj/african_head/african_head.obj");

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuf(width, height, TGAImage::GRAYSCALE);

	light_dir = light_dir.normalize();
	mat4 mv = lookat(eye, center, up);
	std::cout << "ModelView:" << std::endl;
	std::cout << mv << std::endl;
	mat4 pj = projection((eye - center).norm());
	std::cout << "Projection:" << std::endl;
	std::cout << pj << std::endl;
	mat4 vp = viewport(width/8, height/8, width*3/4, height*3/4);
	std::cout << "Viewport:" << std::endl;
	std::cout << vp << std::endl;

	std::cout << vp * pj * mv << std::endl;
	for (int i = 0; i < model->nfaces(); ++i) {
		vec3 screen_coords[3];
		vec3 world_coords[3];
		for (int j = 0; j < 3; ++j) {
			vec3 v = model->vert(i, j);
			world_coords[j] = v;

			vec4 v1 = embed<4>(v, 1.0);
			v1 = vp * pj * mv * v1;
			for (int j = 3; j--; v[j] = v1[j] / v1[3]);
			screen_coords[j].x = v.x;
			screen_coords[j].y = v.y;
			screen_coords[j].z = v.z;
		}
		// vec3 n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]).normalize();
		// float intensity = n * light_dir;
		float intensity[3];
		for (int j = 0; j < 3; ++j) {
			vec3 n = model->normal(i, j).normalize();
			intensity[j] = std::max(0.0, n *  light_dir);
		}
		// if (intensity > 0)
		triangle(screen_coords, zbuf, image, intensity);
	}
	image.write_tga_file("output.tga");
	system("convert output.tga output.png");
	system("mv output.png ../");
	zbuf.write_tga_file("zbuf.tga");
	system("convert zbuf.tga zbuf.png");
	system("mv zbuf.png ../");
}


int main() {
<<<<<<< HEAD
	model = new Model("/home/zhaosiqi/workspace/Projects/renderer/obj/african_head/african_head.obj");
=======
	Model *model = new Model("../obj/african_head/african_head.obj");
>>>>>>> dev

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuf(width, height, TGAImage::GRAYSCALE);
	GouraudShader shader;

	light_dir = light_dir.normalize();
	ModelView = lookat(eye, center, up);
	std::cout << "ModelView:" << std::endl;
	std::cout << ModelView << std::endl;
	Projection = projection((eye - center).norm());
	std::cout << "Projection:" << std::endl;
	std::cout << Projection << std::endl;
	Viewport = viewport(width/8, height/8, width*3/4, height*3/4);
	std::cout << "Viewport:" << std::endl;
	std::cout << Viewport << std::endl;

<<<<<<< HEAD
	std::cout << Viewport * Projection * ModelView << std::endl;
=======
>>>>>>> dev
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
<<<<<<< HEAD
	system("mv zbuf.png /home/zhaosiqi/workspace/Projects/renderer");
=======
	system("mv zbuf.png ../");
	return 0;
>>>>>>> dev
}
