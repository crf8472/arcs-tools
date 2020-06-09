#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"
#endif

extern "C" {
#include <sys/stat.h> // for stat
#include <unistd.h>   // for stat
}

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>  // for ARCS_LOG_DEBUG
#endif

namespace arcsapp
{
namespace file
{

// Define file separator (quick and dirty)
// FIXME C++17's filesystem has better facilites to do this
#ifdef _WIN32
const std::string file_sep("\\");
#else
const std::string file_sep("/");
#endif


std::string path(const std::string &filename)
{
	if (filename.empty())
	{
		return std::string{};
	}

	// FIXME C++17's filesystem has better facilites to do this
	auto pos = filename.find_last_of(file_sep);
	if (pos == std::string::npos) // no file separator found, no path specified
	{
		return std::string{};
	}

	return filename.substr(0, pos).append(file_sep);
}


bool file_exists(const std::string &filename)
{
	// FIXME C-style stuff: use C++17's std::filesystem methods
	// std::filesystem::exists(filename), ...::status and ...::perms

	// Use POSIX stat to check whether file exists
	struct stat buffer;
	return stat(filename.c_str(), &buffer) == 0;
}


void prepend_path(const std::string &path, std::string &filename)
{
	std::string filepath { path };
	auto pos = filepath.find_last_of(file_sep);
	if (pos == std::string::npos) // add trailing file separator
	{
		filepath.append(file_sep);
	}

	filename.insert(0, path);
}


std::string derive_filename(const std::string &filename,
		const std::vector<std::string> &suffices)
{
	if (suffices.empty())
	{
		return std::string();
	}

	// Get basename of reference file (without (last) suffix)

	std::string filename_prefix =
		filename.substr(0, filename.find_last_of("."));
	// TODO C++17 with better alternative?

	std::stringstream ss;
	ss << filename_prefix;

	// Check file existence for the given suffices

	for (const std::string &suffix : suffices)
	{
		ss << '.' << suffix;

		ARCS_LOG_DEBUG << "Check for: '" << ss.str() << "'";

		if (file_exists(ss.str()))
		{
			ARCS_LOG_DEBUG << "Found! Will use this file.";
			return ss.str();
		}

		ss.str("");
	}

	return std::string();
}

} // namespace file
} // namespace arcsapp

