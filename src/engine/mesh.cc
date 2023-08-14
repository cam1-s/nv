#include "mesh.hh"

engine::mesh::mesh() : draw_mode(GL_TRIANGLES), vbo(0), vao(0), ibo(0) {
}

engine::mesh::~mesh() {
}

void engine::mesh::build() {
	if (built) {
		return;
	}

	built = true;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, col));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, nrm));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, uv));

	glGenBuffers(1, &instbo);
	glBindBuffer(GL_ARRAY_BUFFER, instbo);
	glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(glm::mat4), instances.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)16);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)32);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)48);

	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void engine::mesh::draw_instanced() {
	if (instances.size() > 0) {
		if (!built) {
			build();
		}

		if (new_instances) {
			glBindBuffer(GL_ARRAY_BUFFER, instbo);
			glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(glm::mat4), instances.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		glBindVertexArray(vao);
		glDrawArraysInstanced(draw_mode, 0, indices.size(), instances.size());
		glBindVertexArray(0);
	}
}

void engine::mesh::draw() {
	if (!built) {
		built = true;
		build();
	}

	glBindVertexArray(vao);
	glDrawArrays(draw_mode, 0, indices.size());
	glBindVertexArray(0);
}

void engine::mesh::add_instance(glm::mat4 const &m) {
	instances.push_back(m);
	new_instances = true;
}
