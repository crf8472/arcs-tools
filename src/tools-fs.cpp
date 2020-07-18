#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"
#endif

extern "C" {
#include <sys/stat.h> // for stat
#include <unistd.h>   // for stat
}

#include <filesystem> // for exists, status, perms
#include <fstream>    // for ifstream
#include <sstream>    // for ostringstream

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>  // for ARCS_LOG_DEBUG
#endif


namespace arcsapp
{
namespace file
{


std::string path(const std::string &filename)
{
	if (filename.empty())
	{
		return std::string{};
	}

	namespace fs = std::filesystem;

	auto filepath = fs::path(filename);
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

	namespace fs = std::filesystem;

	auto filepath = fs::path(path);
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

	// Commented out: Use POSIX stat to check whether file exists
	//struct stat buffer;
	//return stat(filename.c_str(), &buffer) == 0;
}

} // namespace file
} // namespace arcsapp

