#pragma once
#include "geometry.h"

class Camera {
public:
	Camera() = default;
	Camera(vec3 pos_, vec3 target_, vec3 up_)
		: pos(pos_), target(target_), up(up_) { }
	mat4 get_view_mat();
private:
	vec3 pos;
	vec3 target;
	vec3 up;
};