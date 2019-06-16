#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#define __ARCSTOOLS_TOOLS_FS_HPP__

/**
 * \file
 *
 * \brief Helper functions for the file system
 */

#include <string>
#include <vector>


namespace file
{

/**
 * \brief Returns the path part from the specified filename.
 *
 * The string returned will be empty, if the filename contains no path.
 *
 * \param[in] filename The filename to extract the path from.
 *
 * \return The path, if filename contains any, or an empty string
 */
std::string path(const std::string &filename);

/**
 * \brief Service method: check whether file exists.
 *
 * \param[in] filename The name of the file to check
 *
 * \return TRUE iff this file exists, otherwise FALSE
 */
bool file_exists(const std::string &filename);

/**
 * \brief Service method: check whether a file with given prefix exists with
 * at least one of a list of suffices.
 *
 * If no file with a different suffix exists, the string returned will be
 * empty.
 * This is helpful to derive the name of the metadata file from the audio
 * file and vice versa.
 *
 * \param[in] filename The original filename (prefix and suffix)
 * \param[in] suffices List of suffices to test for
 *
 * \return Name of the existing related file with the first suffix found
 */
std::string derive_filename(const std::string &filename,
		const std::vector<std::string> &suffices);

} // namespace file

#endif

