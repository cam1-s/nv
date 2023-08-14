#include "bsa.hh"
#include "esp.hh"
#include <fstream>
#include <algorithm>
#include <zlib.h>

std::vector<std::string> bsa_names;

struct file_listing {
	std::string name;
	size_t bsa_id;
	size_t offset;
	size_t size;
	bool compressed;
};

struct folder_listing {
	std::string name;
	std::unordered_map<std::string, struct file_listing> file_map;
};

std::unordered_map<std::string, struct folder_listing> folder_map;

void tes4::parse_archive(std::string const &file) {
	// ignoring the hash system for now, i think it would be more efficient to store every name in a map.
	// the hashes seem to be ordered in the file, another option would be binary search. all file names in each
	// archive need to be catalogued first though, so that it knows which archive to search in.

	std::ifstream f(file, std::ios::binary);
	size_t bsa_id = bsa_names.size();
	bsa_names.push_back(file);

	union {
		char data[36];
		struct {
			uint32_t fourcc, version, folder_offset, flags, folder_count, file_count, folder_name_length, file_name_length, type_flags;
		};
	} u;

	f.read(u.data, 36);

	if (u.fourcc != FOURCC('B','S','A',0)) {
		throw std::runtime_error("tes4::parse_archive BSA file with invalid fourcc " + file);
	}

	if (!(u.flags & 1)) {
		throw std::runtime_error("tes4::parse_archive Can't load BSA without embedded folder names " + file);
	}

	if (!(u.flags & 2)) {
		throw std::runtime_error("tes4::parse_archive Can't load BSA without embedded file names " + file);
	}

	if (u.flags & 0x40) {
		throw std::runtime_error("tes4::parse_archive Can't load Xbox archive " + file);
	}

	bool compressed = u.flags & 0x4;

	//std::printf("archive \"%s\"\n\tVersion: %u\n\tFlags: %x\n\tFolders: %u\n\tFiles: %u\n\tType: %x\n\n", file.c_str(), u.version, u.flags, u.folder_count, u.file_count, u.type_flags);
	std::printf("archive \"%s\"\n", file.c_str());

	// folder hash records:
	//  uint64   name hash
	//  uint32   folder file count
	//  uint32   offset to file records

	struct __attribute__((packed)) folder_record {
		uint64_t name_hash;
		uint32_t file_count;
		uint32_t offset;
	};

	std::vector<folder_record> folder_records;
	folder_records.resize(u.folder_count);

	f.read((char *)folder_records.data(), u.folder_count * sizeof(folder_record));

	// file records:
	//   s8cstr  folder name
	//   struct[file_count]:
	//     uint64   file name hash
	//     uint32   file size
	//     uint32   offset

	struct __attribute__((packed)) file_record {
		uint64_t name_hash;
		uint32_t file_size;
		uint32_t offset;
	};

	struct folder_name_record {
		std::string name;
		std::vector<file_record> records;
	};

	std::vector<folder_name_record> folder_name_records;
	folder_name_records.resize(u.folder_count);

	for (uint32_t i = 0; i < u.folder_count; ++i) {
		f.get();
		char name[256];
		for (char *pname = name; (*pname = f.get()); ++pname);
		std::string sname(name);
		folder_name_records[i].name = sname;
		folder_name_records[i].records.resize(folder_records[i].file_count);
		f.read((char*)folder_name_records[i].records.data(), folder_records[i].file_count * sizeof(file_record));
	}

	// file names:
	//   cstr   name
	// for each file in each folder, in the same order as before.

	folder_map.reserve(u.folder_count);

	for (uint32_t i = 0; i < u.folder_count; ++i) {
		auto &fn = folder_name_records[i];

		auto it = folder_map.find(fn.name);
		if (it == folder_map.end())
			it = folder_map.emplace(std::make_pair(fn.name, folder_listing{ fn.name, std::unordered_map<std::string, struct file_listing>() })).first;

		it->second.file_map.reserve(fn.records.size());

		for (uint32_t j = 0; j < fn.records.size(); ++j) {
			char name[256];
			for (char *pname = name; (*pname = f.get()); ++pname);
			std::string sname(name);

			auto &r = fn.records[j];

			size_t size = r.file_size & 0x3fffffff;
			bool comp = compressed;

			if (r.file_size & (1<<30))
				comp = !comp;
			
			it->second.file_map[sname] = file_listing{ sname, bsa_id, r.offset, size, comp };
		}
	}
}

std::vector<uint8_t> tes4::load_file(std::string vfs_path) {
	std::transform(vfs_path.begin(), vfs_path.end(), vfs_path.begin(), ::tolower);
	std::replace(vfs_path.begin(), vfs_path.end(), '/', '\\');

	size_t idx = vfs_path.find_last_of('\\');
	std::string folder_name = vfs_path.substr(0, idx);
	std::string file_name = vfs_path.substr(idx+1);

	//std::printf("loading %s    \\    %s\n", folder_name.c_str(), file_name.c_str());

	auto folder_it = folder_map.find(folder_name);
	if (folder_it == folder_map.end())
		throw std::runtime_error("tes4::load_file can't find file in vfs " + vfs_path);

	auto file_it = folder_it->second.file_map.find(file_name);
	if (file_it == folder_it->second.file_map.end())
		throw std::runtime_error("tes4::load_file can't find file in vfs " + vfs_path);

	std::vector<uint8_t> ret;

	std::ifstream is(bsa_names[file_it->second.bsa_id], std::ios::binary);
	is.seekg(file_it->second.offset, std::ios::beg);
	ret.resize(file_it->second.size);
	is.read((char*)ret.data(), file_it->second.size);

	if (file_it->second.compressed) {
		//std::printf("inflating compressed %s\n", vfs_path.c_str());

		std::vector<uint8_t> ret1;
		ret1.resize(*(uint32_t *)ret.data());

		z_stream strm;
		memset(&strm, 0, sizeof(strm));
		strm.avail_in = ret.size() - sizeof(uint32_t);
		strm.avail_out = ret1.size();
		strm.next_in = ret.data() + sizeof(uint32_t);
		strm.next_out = ret1.data();
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;

		int res = inflateInit(&strm);

		if (res == Z_OK) {
			res = inflate(&strm, Z_FINISH);
			if (res != Z_STREAM_END) {
				inflateEnd(&strm);
				throw std::runtime_error("tes4::load_file zlib inflation failed for file " + vfs_path);
			}
		}
		else {
			inflateEnd(&strm);
			throw std::runtime_error("tes4::load_file zlib inflation failed for file " + vfs_path);
		}

		return ret1;
	}
	else
		return ret;
}
