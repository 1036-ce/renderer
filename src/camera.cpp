#include "camera.h"
#include "gl.h"

mat4 Camera::get_view_mat() {
	return lookat(pos, target, up);
}
