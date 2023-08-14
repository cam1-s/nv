#pragma once

#include <vector>
#include <list>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace engine {

class shader {
public:
	shader(size_t id);
	~shader();

	void add_shader(GLenum type, char const *path);
	void link();

	template <class T>
	void uniform(char const *, T const &);

	size_t id;
	std::list<GLuint> shaders;
	GLuint program;
};

extern std::vector<shader *> shaders;

shader *create_shader();

// called by engine::cleanup
void cleanup_shaders();

}
