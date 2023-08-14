#version 330 core
layout (location = 0) in vec3 in_pos;

uniform mat4 mat_view;
uniform mat4 mat_proj;
uniform mat4 mat_model;

void main() {
	gl_Position = mat_proj * mat_view * mat_model * vec4(in_pos, 1.f);
}
