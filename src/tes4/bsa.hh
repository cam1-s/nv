// utils for bsa files and vfs

#include <string>
#include <vector>

namespace tes4 {

// parse a bsa file
void parse_archive(std::string const &file);
// load a file from the vfs, uncompress if needed
std::vector<uint8_t> load_file(std::string);

}
