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

#include <iostream>                 // for cout, basic_ostream, endl, cerr
#include <fstream>                  // for ofstream
#include <memory>                   // for unique_ptr, allocator
#include <string>                   // for string

#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"        // for CallSyntaxException, Options, Configurator
#endif

namespace arcsapp
{

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
class Application
{
public:

	/**
	 * \brief Default constructor.
	 */
	Application();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Application() noexcept;

	/**
	 * \brief Return the name of the application.
	 *
	 * \return Name of this Application
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
	* \brief Worker: output a result object to file or stdout.
	*
	* The object musst overload operator << for std::ostream or a compile error
	* will occurr.
	*
	* If a filename is specified, the output is directed to the file with the
	* specified name. The default value for \c filename is an empty string which
	* means that the output goes to std::cout.
	*
	* If an existing file is specified, the file is overwritten by default. If
	* TRUE is passed for parameter \c overwrite, the existing file will be
	* reopened and appended to.
	*
	* This function is intended to be used in \c do_run() implementations for
	* results. It is not suited to output errors or log messages.
	*
	* \param[in] object    The object to output
	* \param[in] filename  Optional filename, default is the empty string
	* \param[in] overwrite Iff TRUE, existing files will be overwritten
	*
	* \return Type void iff object overloads operator << for std::ostream
	*/
	template <typename T>
	auto output(T&& object,
			const std::string &filename = std::string{},
			const bool overwrite = true) const
		-> decltype( std::cout << object, void() )
	{
		if (filename.empty())
		{
			std::cout << object;
		} else
		{
			std::ofstream out_file_stream;

			//  overwrite: If file exists, delete it and create new one
			// !overwrite: If file exists, append to it, otherwise create it

			auto mode = std::ios_base::openmode { overwrite
				? std::fstream::out | std::fstream::trunc
				: std::fstream::out | std::fstream::app };

			out_file_stream.open(filename, mode);

			if (!out_file_stream)
			{
				// File does not exist, create it
				out_file_stream.open(filename,
					std::fstream::out | std::fstream::trunc);
			}

			out_file_stream << object;
		}
	}

private:

	/**
	 * \brief Implements name().
	 *
	 * \return Name of this Application
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

