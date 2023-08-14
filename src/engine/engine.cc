#include "engine.hh"
#include "window.hh"
#include "shader.hh"

void engine::init() {
	engine::init_window();
}

void engine::cleanup() {
	engine::cleanup_shaders();
	engine::cleanup_window();
}

