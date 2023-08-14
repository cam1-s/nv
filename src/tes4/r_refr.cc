#include "r_refr.hh"

#include <GL/glew.h>
#include <glm/ext/matrix_transform.hpp>

tes4::r_refr::r_refr(tes4::record_info *i) : actor(), data(0) {
	tes4::managed_data d = tes4::form_data(i->formid);

	for (size_t offs = 0; offs < d.size;) {
		tes4::field f = tes4::next_field(d, offs);

		switch (f.type) {
		case FOURCC('N','A','M','E'):
			data = new tes4::form_ref<tes4::r_stat>(*(uint32_t *)f.data);
			break;
		case FOURCC('D','A','T','A'):
			pos = *(glm::vec3 *)f.data;
			rot = *(glm::vec3 *)(f.data + 3 * sizeof(float));
			break;
		}

		//if (f.fourcc == "NAME") {
		//	auto *name = tes4::get_form(*(uint32_t *)f.data);
			//if (name->fourcc == "STAT") {
			//	
			//}
		//}


		//char *str = (char *)&f.type;
		//std::printf("field %c%c%c%c\n", str[0], str[1], str[2], str[3]);
	}
}

tes4::r_refr::~r_refr() {
	if (data)
		delete data;
}

void tes4::r_refr::draw(engine::shader *shader) {
	glm::mat4 model(1.f);
	model = glm::translate(model, pos);
	shader->uniform("mat_model", model);
	(*data)->draw();
}
