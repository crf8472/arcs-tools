#ifndef __ARCSTOOLS_VERSION_HPP___
#define __ARCSTOOLS_VERSION_HPP___

/**
 * \file
 *
 * \brief Provide version and name information for arcs-tools.
 */

#include <string>

/**
 * \brief Classes for arcstk and its applications.
 */
namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief Name of the binary
 */
extern const std::string ARCSTOOLS_BINARY_NAME;

/**
 * \brief Major version number.
 */
extern const int ARCSTOOLS_VERSION_MAJOR;

/**
 * \brief Minor version number.
 */
extern const int ARCSTOOLS_VERSION_MINOR;

/**
 * \brief Patch level.
 */
extern const int ARCSTOOLS_VERSION_PATCH;

/**
 * \brief Version suffix (e.g. '-prealpha1', '-beta', '-rc3' etc.)
 */
extern const std::string ARCSTOOLS_VERSION_SUFFIX;

/**
 * \brief Complete semantic version information
 */
extern const std::string ARCSTOOLS_VERSION;

/**
 * \brief Version info as shown by `git describe --always HEAD`.
 */
extern const std::string ARCSTOOLS_GIT_VERSION;


/**
 * \internal
 * \brief Namespace for implementation details.
 */
namespace details
{
// empty
}

} // namespace v_1_0_0
} // namespace arcsapp

#endif


