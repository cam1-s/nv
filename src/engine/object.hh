#pragma once

#include <vector>

namespace engine {

class mesh;

// GL textured object
// can contain multiple meshes.
// safe to create on different thread
class object {
public:
	object();
	~object();

	void draw();
	mesh &add_mesh();
	
	std::vector<mesh *> meshes;
};

}
