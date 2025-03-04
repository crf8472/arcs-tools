#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#define __ARCSTOOLS_TOOLS_FS_HPP__

/**
 * \file
 *
 * \brief Helper functions for the file system
 */

#include <string> // for string

namespace arcsapp
{
inline namespace v_1_0_0
{
/**
 * \brief Tools and helpers for accessing the file system.
 */
namespace file
{

/**
 * \brief Returns the path part from the specified filename.
 *
 * The string returned will be empty, if the filename contains no path.
 *
 * The returned string is guaranteed to end with a path separator.
 *
 * \param[in] filename The filename to extract the path from.
 *
 * \return The path, if filename contains any, or an empty string
 */
std::string path(const std::string& filename);

/**
 * \brief Worker to prepend a path to a filename.
 *
 * Parameter \c path must end with a file separator or its last part will be
 * replaced by \c filename.
 *
 * \param[in]     path     The (absolute or relative) path to prepend
 * \param[in,out] filename The filename to prepend \c path to
 */
void prepend_path(const std::string& path, std::string& filename);

/**
 * \brief Service method: check whether file exists and is readable.
 *
 * TRUE, if owner, group and others are allowed to read the file (i.e.
 * file must at least have 0444).
 *
 * \param[in] filename The name of the file to check
 *
 * \return TRUE iff this file exists and is readable, otherwise FALSE
 */
bool file_is_readable(const std::string& filename);

} // namespace file
} // namespace v_1_0_0
} // namespace arcsapp

#endif

