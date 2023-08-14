#include "shader.hh"

#include <fstream>

std::vector<engine::shader *> engine::shaders;

engine::shader::shader(size_t i) : id(i), program(0) {
}

void engine::shader::add_shader(GLenum type, char const *path) {
	std::ifstream f(path, std::ios::binary);
	std::vector<char> fc;
	f.seekg(0, std::ios::end);
	GLint size = f.tellg();
	fc.resize(size);
	f.seekg(0, std::ios::beg);
	f.read(fc.data(), size);

	GLuint shader = glCreateShader(type);
	char *data = fc.data();
	glShaderSource(shader, 1, &data, &size);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (!status) {
		char log[512];
		glGetShaderInfoLog(shader, 512, NULL, log);
		std::printf("Compilation of shader \"%s\" failed:\n%s", path, log);
	}

	shaders.push_back(shader);
}

void engine::shader::link() {
	program = glCreateProgram();

	for (auto i : shaders)
		glAttachShader(program, i);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (!status) {
		char log[512];
		glGetProgramInfoLog(program, 512, NULL, log);
		std::printf("Linking of program %" PRIuPTR " failed:\n%s", id, log);
	}
	
	while (shaders.size()) {
		glDeleteShader(shaders.back());
		shaders.pop_back();
	}
}

engine::shader::~shader() {
	while (shaders.size()) {
		glDeleteShader(shaders.back());
		shaders.pop_back();
	}
	glDeleteProgram(program);
}

engine::shader *engine::create_shader() {
	engine::shader *s = new engine::shader(shaders.size());
	engine::shaders.push_back(s);
	return s;
}

void engine::cleanup_shaders() {
	for (auto *i : engine::shaders)
		delete i;
}

template <>
void engine::shader::uniform(char const *loc, glm::mat4 const &d) {
	glUniformMatrix4fv(glGetUniformLocation(program, loc), 1, false, &d[0][0]);
}
