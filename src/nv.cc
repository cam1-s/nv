#include "tes4/esp.hh"
#include "tes4/bsa.hh"
#include "tes4/r_refr.hh"
#include "engine/engine.hh"
#include "engine/camera.hh"
#include "engine/shader.hh"
#include "engine/window.hh"

int main() {
	engine::init();

	tes4::parse_plugin("exedata.esm");
	tes4::parse_plugin("FalloutNV.esm");
	//std::printf("loaded plugins. %llu forms.\n", tes4::forms.size());

	tes4::parse_archive("Fallout - Meshes.bsa");
	tes4::parse_archive("Fallout - Textures.bsa");
	tes4::parse_archive("Fallout - Textures2.bsa");
	
	tes4::grup_node *wrld = tes4::get_grup("WRLD");
	tes4::grup_node *level = wrld->grups[0xDA726]; // WastelandNV

	std::list<tes4::form_ref<tes4::r_refr>> refrs;

	for (auto pi : level->grups) {
		tes4::grup_node *i = pi.second;
		//std::printf("%08x, %08x\n", i->label, i->type);

		if (i->type == 4) { // exterior cell block
			std::printf("Block %d %d\n", (int16_t)i->label, (int16_t)(i->label >> 16));

			for (auto pj : i->grups) {
				tes4::grup_node *j = pj.second;

				if (j->type == 5) { // exterior cell sub-block
					//std::printf("  Sub-Block %d %d\n", (int16_t)j->label, (int16_t)(j->label >> 16));

					for (auto pk : j->grups) {
						tes4::grup_node *k = pk.second;

						if (k->type == 6) { // cell children (label = formid of cell)
							for (auto pl : k->grups) {
								tes4::grup_node *l = pl.second;

								for (uint32_t m : l->formids) {
									tes4::record_info *f = tes4::get_form(m);
									
									if (f->type == FOURCC('R','E','F','R')) {
										refrs.push_back(tes4::form_ref<tes4::r_refr>(m));
									}
								}
							}
						}
					}
					//break;
				}
			}
			//break;
		}

	}

	engine::camera c;
	c.perspective(60, 800.f/600.f, .6f, 1000.f);

	engine::shader *s = engine::create_shader();
	s->add_shader(GL_VERTEX_SHADER, "object_vert.glsl");
	s->add_shader(GL_FRAGMENT_SHADER, "object_frag.glsl");

	std::printf("%" PRIuPTR " draws\n", engine::actors.size());

	glPointSize(5.f);

	while (!engine::window_closed()) {
		engine::update_window();

		glClearColor(.5f, .5f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(s->program);
		s->uniform("mat_proj", c.proj);
		s->uniform("mat_view", c.view);

		for (auto i : engine::actors)
			i->draw(s);
	}

	engine::cleanup();
	
	return 0;
}
