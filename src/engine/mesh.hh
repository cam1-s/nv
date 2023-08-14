#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

namespace engine {

struct vertex {
	glm::vec3 pos = glm::vec3(0.f);
	glm::vec4 col = glm::vec4(1.f);
	glm::vec3 nrm = glm::vec3(0.f,1.f,0.f);
	glm::vec2 uv = glm::vec2(0.f);
};

class mesh {
public:
	mesh();
	~mesh();
	
	void build();
	void draw_instanced();
	void draw();
	void add_instance(glm::mat4 const &m);

	std::vector<vertex> vertices;
	std::vector<glm::mat4> instances;
	std::vector<uint32_t> indices;

	GLenum draw_mode;
	GLuint vbo;
	GLuint vao;
	GLuint ibo;
	GLuint instbo;

	bool built;
	bool new_instances;
};

}
