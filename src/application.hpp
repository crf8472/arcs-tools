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

#include <fstream>     // for ofstream
#include <ios>         // for ios_base
#include <iostream>    // for cout
#include <memory>      // for unique_ptr, allocator
#include <mutex>       // for mutex, lock_guard
#include <string>      // for string

namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief An output stream.
 */
class Output final
{
public:

	/**
	 * \brief Default constructor.
	 */
	Output();

	/**
	 * \brief TRUE iff output appends to previous output.
	 *
	 * This is only relevant for file output.
	 *
	 * \return TRUE otherwise FALSE
	 */
	bool is_appending() const;

	/**
	 * \brief Set whether further output will be appended to previous output.
	 *
	 * Iff set to TRUE, further output will not overwrite previous output but
	 * just append.
	 *
	 * This is only relevant for file output.
	 *
	 * \param[in] append Flag for appending
	 */
	void set_append(const bool append);

	/**
	 * \brief Name of the output file.
	 *
	 * \return Name of file to output to
	 */
	const std::string& filename() const;

	/**
	 * \brief Set output file.
	 *
	 * \param[in] filename  Name of file to output to
	 */
	void to_file(const std::string& filename);

	/**
	* \brief Worker: output a result object to file or stdout.
	*
	* \details
	*
	* The object musst overload operator << for std::ostream or a compile error
	* will occurr.
	*
	* If a filename is specified, the output is directed to the file with the
	* specified name. If <tt>filename()</tt> returns an empty string means that
	* the output is passed to <tt>std::cout</tt>.
	*
	* If an existing file is specified, the file is overwritten by default.
	* This behaviour can be changed by <tt>set_append(true)</tt> before calling
	* <tt>output()</tt>.
	*
	* This function is intended to be used in <tt>do_run()</tt> implementations
	* for results. It is not suited to output errors or log messages.
	*
	* \param[in] object    The object to output
	*
	* \return Type void iff object overloads operator << for std::ostream
	*/
	template <typename T>
	inline auto output(T&& object) -> decltype( std::cout << object, void() )
	{
		const std::lock_guard<std::mutex> lock(mutex_);

		if (filename().empty())
		{
			std::cout << object;
			return;
		}

		// write to file

		std::ofstream out_file_stream;

		auto mode = std::ios_base::openmode { is_appending()
			? std::fstream::out | std::fstream::app
			: std::fstream::out | std::fstream::trunc };

		out_file_stream.open(filename(), mode);

		if (!out_file_stream)
		{
			// File does not exist, create it
			out_file_stream.open(filename(),
				std::fstream::out | std::fstream::trunc);
		}

		out_file_stream << object;

		append_ = true; // first call overwrites, subsequent calls append
	}

	/**
	 * \brief Acquire singleton instance.
	 */
	static Output& instance();

private:

	/**
	 * \brief Internal guard.
	 */
	std::mutex mutex_;

	/**
	 * \brief Internal output filename.
	 */
	std::string filename_;

	/**
	 * \brief Internall append flag.
	 */
	bool append_;
};


// forward declare instead of include
class Configurator;
class Configuration;
class Options;
class Result;


/**
 * \brief Abstract base class for command line applications.
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

	/**
	 * \brief Print version information to std::cout.
	 */
	void print_version() const;

protected:

	/**
	 * \brief Create a Configurator for this instance.
	 *
	 * \return The configurator for this application
	 */
	std::unique_ptr<Configurator> create_configurator() const;

	/**
	 * \brief Setup logging according to options.
	 */
	void setup_logging(const Options& options) const;

	/**
	 * \brief Generates a fatal error with specified message.
	 *
	 * The error message will go to stderr and as well to the internal logging.
	 *
	 * \param[in] message The error message
	 */
	void fatal_error(const std::string& message) const;

	/**
	 * \brief Output the result.
	 *
	 * \param[in] result  Result object to output
	 */
	void output(std::unique_ptr<Result> result) const;

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
	virtual std::unique_ptr<Configurator> do_create_configurator() const
	= 0;

	/**
	 * \brief Implements run().
	 *
	 * \param[in] config The configuration to run the application
	 *
	 * \return Application return code
	 */
	virtual int do_run(const Configuration& config)
	= 0;
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

