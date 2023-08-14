#include "r_stat.hh"
#include "bsa.hh"
#include "nif.hh"

tes4::r_stat::r_stat(tes4::record_info *i) {
	tes4::managed_data d = tes4::form_data(i->formid);

	for (size_t offs = 0; offs < d.size;) {
		tes4::field f = tes4::next_field(d, offs);

		switch (f.type) {
		case FOURCC('M','O','D','L'):
			try {
				std::string sname = std::string((char *)f.data);
				tes4::load_nif(tes4::load_file("meshes\\" + sname), sname, obj);
			}
			catch (std::runtime_error &e) {
				std::printf("%s\n", e.what());
			}
			//std::printf("%.4s %s\n", &i->type, f.data);
			break;
		}
	}
}

tes4::r_stat::~r_stat() {
}

void tes4::r_stat::draw() {
	obj.draw();
}
