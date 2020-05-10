#ifndef __ARCSTOOLS_APPLICATION_HPP__
#define __ARCSTOOLS_APPLICATION_HPP__

#include <iostream>
#include <fstream>

/**
 * \file
 *
 * \brief Base interface for command line applications.
 *
 * Provides an interface for named, configurable applications with access to
 * command line input.
 */

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include <arcsdec/descriptors.hpp>
#endif

#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif


using arcsdec::FileReaderDescriptor;


/**
 * \brief Output an object that overloads operator << for std::ostream
 *
 * If a filename is specified, the output is directed to the file with the
 * specified name. The default value for filename is "" which means that
 * the output goes to std::cout.
 *
 * \param[in] object   The object to print
 * \param[in] filename Optional filename, default is ""
 */
template <typename T>
inline auto output(const T &object, const std::string &filename = "")
	-> decltype( std::cerr << object, void() )
{
	if (filename.empty())
	{
		std::cout << object;
	} else
	{
		std::ofstream out_file_stream;
		out_file_stream.open(filename);

		out_file_stream << object;
	}
}


/**
 * \brief Collect descriptor infos
 */
class FormatCollector
{
public:

	FormatCollector();

	/**
	 * \brief Add information represented by a descriptor
	 *
	 * \param[in] descriptor Add the info from this descriptor
	 */
	void add(const FileReaderDescriptor &descriptor);

	/**
	 * \brief Get the collected information
	 *
	 * \return Information collected
	 */
	std::vector<std::array<std::string, 4>> info() const;

private:

	std::vector<std::array<std::string, 4>> info_;
};


/**
 * \brief Abstract base class for AR tools command line applications.
 *
 * Implements parsing of the command line input, configuration of logging and
 * creation of Options.
 *
 * Defines an interface to create a configurator, run the application or
 * print a usage message.
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
	 * \return The result value of the application
	 */
	int run(int argc, char** argv);

	/**
	 * \brief Print usage information.
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

private:

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
	 * \brief Implements name().
	 *
	 * \return Name of this ARApplication
	 */
	virtual std::string do_name() const
	= 0;

	/**
	 * \brief Implements run().
	 *
	 * \param[in] options The options for this application
	 *
	 * \return The result value of the application
	 */
	virtual int do_run(const Options &options)
	= 0;

	/**
	 * \brief Implements print_usage().
	 */
	virtual void do_print_usage() const
	= 0;
};

#endif

