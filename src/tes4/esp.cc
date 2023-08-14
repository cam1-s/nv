#include "esp.hh"

#include <cstring>
#include <cinttypes>
#include <fstream>

std::vector<tes4::plugin_buffer *> tes4::plugins;
std::unordered_map<uint32_t, tes4::record_info *> tes4::forms;
tes4::grup_node tes4::grups;

#define parse_plugin_r_read32() *(uint32_t *)(f + offs); offs += sizeof(uint32_t)

void parse_plugin_r(int plugin_id, uint8_t *f, size_t &offs, size_t limit, tes4::grup_node *gn=&tes4::grups, size_t depth=0) {
	// Record:
	//   Type     char[4]
	//   Size     uint32
	//   Flags    uint32
	//   FormID   uint32
	//   Version  uint32
	//   Revision uint32
	//   Data     char[Size]

	// Grup:
	//   Type     char[4] == "GRUP"
	//   Size     uint32
	//   Label    char[4]
	//   Type     uint32
	//   Version  uint32
	//   Revision uint32
	//   Data     char[Size - 20]

	std::string dstr(depth, ' ');

	for (;;) {
		size_t pos = offs;

		if (pos >= limit)
			break;

		uint32_t type;
		uint32_t size;
		uint32_t flags; // label for GRUP
		uint32_t formid; // type for GRUP

		type = parse_plugin_r_read32();
		size = parse_plugin_r_read32();
		flags = parse_plugin_r_read32();
		formid = parse_plugin_r_read32();
		offs += sizeof(uint32_t) * 2; // skip version and revision, i dont care about them.

		if (!strncmp("GRUP", (char *)&type, sizeof(uint32_t))) {
			//char *a = (char *)&type;
			//char *b = (char *)&flags;
			//if (formid)
			//	std::printf("0x%08" PRIx64 "  :  %stype: %c%c%c%c, size: 0x%08x, type: 0x%08x, label: 0x%08x\n", pos, dstr.c_str(), a[0],a[1],a[2],a[3], size, formid, flags);
			//else
			//	std::printf("0x%08" PRIx64 "  :  %stype: %c%c%c%c, size: 0x%08x, type: 0x%08x, label: %c%c%c%c\n", pos, dstr.c_str(), a[0],a[1],a[2],a[3], size, formid, b[0],b[1],b[2],b[3]);
			
			tes4::grup_node *n;

			auto it = gn->grups.find(flags);

			if (it != gn->grups.end()) {
				n = it->second;
			}
			else {
				n = new tes4::grup_node{ flags, formid, std::unordered_map<uint32_t, tes4::grup_node *>(), std::set<uint32_t>() };
				gn->grups[flags] = n;
			}

			parse_plugin_r(plugin_id, f, offs, pos + size - 24, n, depth + 1);
		}
		else {
			gn->formids.insert(formid);
			tes4::forms[formid] = new tes4::record_info{ plugin_id, pos, type, size, flags, formid, 0, 0, std::mutex() };
			offs += size;
		}
	}
}

void tes4::parse_plugin(std::string const &file) {
	std::printf("plugin \"%s\"\n", file.c_str());

	int plugin_id = plugins.size();

	std::ifstream fs(file, std::ios::binary);

	plugins.push_back(new tes4::plugin_buffer{ file, std::vector<uint8_t>() });

	fs.seekg(0, std::ios::end);
	size_t s = fs.tellg();
	plugins.back()->data.resize(s);
	fs.seekg(0, std::ios::beg);
	fs.read((char *)plugins.back()->data.data(), s);

	size_t offs = 0;
	parse_plugin_r(plugin_id, plugins.back()->data.data(), offs, s);
}

tes4::record_info *tes4::get_form(uint32_t id) {
	auto it = tes4::forms.find(id);
	
	if (it == tes4::forms.end()) {
		char buf[50];
		snprintf(buf, 50, "tes4::get_form can't find form with id %06X\n", id);
		throw std::runtime_error(buf);
	}
	else {
		return it->second;
	}
}

tes4::grup_node *tes4::get_grup(char const *name, tes4::grup_node *node) {
	uint32_t u = name[0] | (name[1] << 8) | (name[2] << 16) | (name[3] << 24);
	auto it = node->grups.find(u);

	if (it != node->grups.end()) {
		return it->second;
	}
	else {
		throw std::runtime_error(std::string("tes4::get_grup can't find grup \"") + name + "\"");
	}
}

void cleanup_grup_tree(tes4::grup_node *gn=&tes4::grups) {
	for (auto i : gn->grups) {
		cleanup_grup_tree(i.second);
		delete i.second;
	}
}

void __attribute__((destructor)) tes4_cleanup() {
	for (auto *i : tes4::plugins)
		delete i;

	for (auto i : tes4::forms)
		delete i.second;

	cleanup_grup_tree();
}

tes4::managed_data tes4::form_data(uint32_t formid) {
	// TODO: uncompress
	tes4::record_info *r = tes4::get_form(formid);
	return tes4::managed_data{ tes4::plugins[r->plugin]->data.data() + r->offset + 24, r->size - 24, false };
}

tes4::field tes4::next_field(tes4::managed_data &data, size_t &offs) {
	// TODO: check for XXXX
	uint32_t type = *(uint32_t *)(data.data + offs); offs += sizeof(uint32_t);
	uint16_t size = *(uint16_t *)(data.data + offs); offs += sizeof(uint16_t);

	tes4::field f{ type, size, data.data + offs };
	offs += size;
	return f;
}
