#pragma once

#include <glm/glm.hpp>
#include <list>
#include "shader.hh"

namespace engine {

class actor;

extern std::list<actor *> actors;

class actor {
public:
	actor() : pos(0.f), rot(0.f), scale(1.f) {
		actors.push_front(this);
		actlist_it = actors.begin();
	}

	virtual ~actor() {
		actors.erase(actlist_it);
	}

	virtual void draw(shader *) = 0;
	glm::vec3 pos, rot, scale;

private:
	std::list<actor *>::iterator actlist_it;
};

}
