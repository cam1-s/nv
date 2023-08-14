// esm/esp file parser

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <cstring>
#include <mutex>

// helper for comparing ints to fourcc
#define FOURCC(a,b,c,d) (a|(b<<8)|(c<<16)|(d<<24))

namespace tes4 {

// parse an esp or esm file and
void parse_plugin(std::string const &file);

struct plugin_buffer {
	std::string name;
	std::vector<uint8_t> data;
};

extern std::vector<plugin_buffer *> plugins;

struct record_info {
	int plugin;
	size_t offset;
	uint32_t type;
	uint32_t size;
	uint32_t flags;
	uint32_t formid;
	size_t ref_count;
	void *ref_data;
	std::mutex ref_mutex;
};

extern std::unordered_map<uint32_t, record_info *> forms;

// get a form by id (will throw)
record_info *get_form(uint32_t id);

struct grup_node {
	uint32_t label;
	uint32_t type;
	std::unordered_map<uint32_t, grup_node *> grups;
	std::set<uint32_t> formids;
};

extern grup_node grups;

// get a named grup.
grup_node *get_grup(char const *name, grup_node *node=&grups);

// frees data if allocated
class managed_data {
public:
	managed_data(uint8_t *d, size_t s, bool f) :
		data(d), size(s), should_free(f) {  }
	~managed_data() {
		if (should_free)
			delete data;
	}

	uint8_t *data;
	size_t size;
	bool should_free;
};

managed_data form_data(uint32_t formid);

struct field {
	uint32_t type;
	size_t data_size;
	uint8_t *data;
};

field next_field(managed_data &data, size_t &offs);

class form_ref_untyped {
public:
	virtual ~form_ref_untyped() {};
};

// managed forms
// TODO: unload timer, unload on background thread
template <class form_parser>
class form_ref : public form_ref_untyped {
public:
	form_ref() {
		info = 0;
	}

	form_ref(uint32_t id) {
		info = tes4::get_form(id);
		std::lock_guard guard(info->ref_mutex);

		if (info->ref_count == 0) {
			info->ref_data = new form_parser(info);
		}

		info->ref_count++;
	}

	form_ref(form_ref const &rhs) {
		operator=(rhs);
	}

	~form_ref() {
		if (info) {
			std::lock_guard guard(info->ref_mutex);

			info->ref_count--;
			if (info->ref_count == 0) {
				delete (form_parser *)info->ref_data;
			}
		}
	}

	form_ref &operator=(form_ref const &rhs) {
		info = rhs.info;
		info->ref_count++;
		return *this;
	}

	inline form_parser *operator->() {
		return (form_parser *)info->ref_data;
	}

private:
	record_info *info;
};

}
