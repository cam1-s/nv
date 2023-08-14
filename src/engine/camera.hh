#pragma once

#include <glm/glm.hpp>

namespace engine {

class camera {
public:
	camera();
	void perspective(float fov, float aspect, float near, float far);
	void lookat(glm::vec3 const &from, glm::vec3 const &to, glm::vec3 const &up);

	glm::mat4 proj, view;

	// update from pos and viewdir
	void update();

	glm::vec3 pos;
	glm::vec3 viewdir;
};

}
