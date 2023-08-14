#pragma once

#include "esp.hh"
#include "../engine/object.hh"

namespace tes4 {

class r_stat {
public:
	r_stat(record_info *i);
	~r_stat();

	void draw();

	engine::object obj;
};

}
