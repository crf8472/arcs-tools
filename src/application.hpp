#ifndef __ARCSTOOLS_APPLICATION_HPP__
#define __ARCSTOOLS_APPLICATION_HPP__

/**
 * \file
 *
 * \brief Interface and abstract base for command line applications.
 *
 * Provides an interface for named, configurable applications with access to
 * command line input.
 */

#include <array>                    // for array
#include <cstdlib>                  // for EXIT_FAILURE
#include <iostream>                 // for cout, basic_ostream, endl, cerr
#include <fstream>                  // for ofstream
#include <memory>                   // for unique_ptr, allocator
#include <string>                   // for string
#include <vector>                   // for vector

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include <arcsdec/descriptors.hpp>  // for FileReaderDescriptor
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"          // for StringTable
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif

namespace arcsapp
{

class Configurator;


/**
 * \brief Abstract base class for command line applications.
 *
 * Defines an interface to create a delegate that creates the configuration
 * object and run the application.
 *
 * Implements parsing of the command line input to a configuration object,
 * configuration of logging and the core \c run() function. Provides workers
 * for fatal_error() and output() as well as printing the usage info.
 */
class ARApplication
{
public:

	/**
	 * \brief Default constructor.
	 */
	ARApplication();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARApplication() noexcept;

	/**
	 * \brief Return the name of the application.
	 *
	 * \return Name of this ARApplication
	 */
	std::string name() const;

	/**
	 * \brief Run this application with the given command line input.
	 *
	 * \param[in] argc Number of command line input arguments
	 * \param[in] argv Array of command line input arguments
	 *
	 * \return Application return code
	 */
	int run(int argc, char** argv);

	/**
	 * \brief Print usage information to std::cout.
	 */
	void print_usage() const;

protected:

	/**
	 * \brief Setup the Options for this application.
	 *
	 * \param[in] argc Number of command line input arguments
	 * \param[in] argv Array of command line input arguments
	 *
	 * \return Options instance representing the command line input
	 */
	std::unique_ptr<Options> setup_options(int argc, char** argv) const;

	/**
	 * \brief Generates a fatal error with specified message.
	 *
	 * The error message will go to stderr and as well to the internal logging.
	 *
	 * \param[in] message The error message
	 */
	void fatal_error(const std::string &message) const;

	/**
	* \brief Worker: output a result object
	*
	* The object musst overload operator << for std::ostream or a compile error
	* will occurr.
	*
	* If a filename is specified, the output is directed to the file with the
	* specified name. The default value for \c filename is an empty string which
	* means that the output goes to std::cout.
	*
	* If an existing file is specified, the file be reopened and appended to.
	*
	* This function is intended to be used in \c do_run() implementations for
	* results. It is not suited to output errors.
	*
	* \param[in] object   The object to output
	* \param[in] filename Optional filename, default is ""
	*
	* \return Type void iff object overloads operator << for std::ostream
	*/
	template <typename T>
	auto output(T&& object, const std::string &filename = std::string{}) const
		-> decltype( std::cout << object, void() )
	{
		if (filename.empty()) // THIS defines the default behaviour!
		{
			std::cout << object;
		} else
		{
			std::ofstream out_file_stream;
			out_file_stream.open(filename);

			out_file_stream << object;
		}
	}

private:

	/**
	 * \brief Implements name().
	 *
	 * \return Name of this ARApplication
	 */
	virtual std::string do_name() const
	= 0;

	/**
	 * \brief Used in print_usage().
	 */
	virtual std::string do_call_syntax() const
	= 0;

	/**
	 * \brief Create a Configurator for this instance.
	 *
	 * \return The configurator for this application
	 */
	virtual std::unique_ptr<Configurator> create_configurator() const
	= 0;

	/**
	 * \brief Create a Configurator for this instance.
	 *
	 * \param[in] argc Number of command line input arguments
	 * \param[in] argv Array of command line input arguments
	 *
	 * \return The configurator for this application
	 */
	virtual std::unique_ptr<Configurator> create_configurator(
			int argc, char** argv) const
	= 0;

	/**
	 * \brief Implements run().
	 *
	 * \param[in] options The options to run the application
	 *
	 * \return Application return code
	 */
	virtual int do_run(const Options &options)
	= 0;
};

} // namespace arcsapp

#endif

