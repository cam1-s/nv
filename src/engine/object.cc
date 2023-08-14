#include "object.hh"
#include "mesh.hh"

engine::object::object() {
}

engine::object::~object() {
	for (auto &i : meshes)
		delete i;
}

engine::mesh &engine::object::add_mesh() {
	engine::mesh *m = new mesh();
	meshes.push_back(m);
	return *m;
}

void engine::object::draw() {
	for (auto i : meshes)
		i->draw();
}
