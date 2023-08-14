#include "camera.hh"

#include <glm/gtx/transform.hpp>

engine::camera::camera() {
}

void engine::camera::perspective(float fov, float aspect, float near, float far) {
	proj = glm::perspective(fov, aspect, near, far);
}

void engine::camera::lookat(glm::vec3 const &from, glm::vec3 const &to, glm::vec3 const &up) {
	view = glm::lookAt(from, to, up);
}

void engine::camera::update() {
	lookat(pos, pos + viewdir, glm::vec3(0.f, 1.f, 0.f));
}
