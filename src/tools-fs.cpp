#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"
#endif

#include <filesystem> // for exists, status, perms
#include <fstream>    // for ifstream
#include <string>     // for string


namespace arcsapp
{
inline namespace v_1_0_0
{
namespace file
{


std::string path(const std::string &filename)
{
	if (filename.empty())
	{
		return std::string{};
	}

	auto filepath = std::filesystem::path(filename);
	filepath.remove_filename();
	filepath.make_preferred();

	return filepath.string();
}


void prepend_path(const std::string &path, std::string &filename)
{
	if (filename.empty())
	{
		filename = path;
		return;
	}

	auto filepath = std::filesystem::path(path);
	filepath.replace_filename(filename);
	filepath.make_preferred();

	filename = filepath.string();
}


bool file_is_readable(const std::string &filename)
{
	auto open_file = std::ifstream { filename };

	if (open_file)
	{
		open_file.close();
		return true;
	}
	return false;
}

} // namespace file
} // namespace v_1_0_0
} // namespace arcsapp

