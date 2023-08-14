#pragma once

#include "esp.hh"
#include "r_stat.hh"
#include "../engine/actor.hh"
#include <glm/glm.hpp>

namespace tes4 {

class r_refr : public engine::actor {
public:
	r_refr(record_info *i);
	~r_refr();
	void draw(engine::shader *);
	tes4::form_ref<tes4::r_stat> *data;
};

}
