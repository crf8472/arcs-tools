#ifndef __ARCSTOOLS_OPTIONS_HPP__
#define __ARCSTOOLS_OPTIONS_HPP__

/**
 * \file
 *
 * \brief Options for runtime configuration.
 *
 * The Options class represents the complete command line input to an
 * application. Options are produced by a Configurator.
 *
 * A single Option instance represents a supported command line option.
 */

#include <climits>   // CHAR_BIT
#include <map>       // for map
#include <string>    // for string
#include <vector>    // for vector

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"           // for OptionCode
#endif

namespace arcsapp
{

class Options;
std::ostream& operator << (std::ostream& out, const Options &options);


/**
 * \brief Base class for configuration options.
 *
 * An Options object contains the complete configuration information for an
 * ARApplication.
 */
class Options
{
public:

	friend std::ostream& operator << (std::ostream& out, const Options &options);

	/**
	 * \brief Options with predefined number of flags.
	 */
	Options(const std::size_t size);

	/**
	 * \brief Default constructor.
	 */
	Options() : Options(sizeof(OptionCode) * CHAR_BIT) { /* empty */ };

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Options() noexcept;

	/**
	 * \brief Set or unset the help flag.
	 *
	 * The help flag indicates whether the option HELP was passed.
	 *
	 * \param[in] help The flag to set or unset
	 */
	void set_help(const bool help);

	/**
	 * \brief Return the help flag.
	 *
	 * \return TRUE iff the help option is set, otherwise FALSE
	 */
	bool is_set_help() const;

	/**
	 * \brief Set or unset the version flag.
	 *
	 * The version flag indicates whether the option VERSION was passed.
	 *
	 * \param[in] version The flag to set or unset
	 */
	void set_version(const bool version);

	/**
	 * \brief Return the version flag.
	 *
	 * \return TRUE iff the version option is set, otherwise FALSE
	 */
	bool is_set_version() const;

	/**
	 * \brief Set the name of the output file.
	 *
	 * \param[in] output Name of the output file
	 */
	void set_output(const std::string &output);

	/**
	 * \brief Return the name of the output file.
	 *
	 * \return Name of the output file
	 */
	std::string output() const;

	/**
	 * \brief Inherited worker to implement getters for option checks.
	 *
	 * \param[in] option The option to check for
	 *
	 * \return TRUE iff the option is set, otherwise FALSE
	 */
	bool is_set(const OptionCode &option) const;

	/**
	 * \brief Set the option to TRUE.
	 *
	 * If the option is currently set, the call has no effect.
	 *
	 * \param[in] option The option to be set to TRUE
	 */
	void set(const OptionCode &option);

	/**
	 * \brief Set the option to FALSE.
	 *
	 * If the option is currently unset, the call has no effect.
	 *
	 * \param[in] option The option to be set to FALSE
	 */
	void unset(const OptionCode &option);

	/**
	 * \brief Get option value by key.
	 *
	 * \param[in] option The option whose value to get
	 *
	 * \return The value of the option passed
	 */
	std::string get(const OptionCode &option) const;

	/**
	 * \brief Set option value for key.
	 *
	 * \param[in] option The option to put in
	 * \param[in] value  The value for the option to put in
	 */
	void put(const OptionCode &option, const std::string &value);

	/**
	 * \brief Get all input options.
	 *
	 * \return All input arguments
	 */
	std::vector<bool> const get_flags() const;

	/**
	 * \brief Get all input arguments.
	 *
	 * \return All input arguments
	 */
	std::vector<std::string> const get_arguments() const;

	/**
	 * \brief Get an input argument by index.
	 *
	 * Will return the \c i-th argument inserted on command line.
	 *
	 * \param[in] i Index of the argument in the argument list
	 *
	 * \return Argument
	 */
	std::string const get_argument(const OptionCode &i) const;

	/**
	 * \brief Returns TRUE iff no arguments are present.
	 *
	 * \return TRUE iff no arguments are present otherwise FALSE
	 */
	bool no_arguments() const;

	/**
	 * \brief Puts an argument to the end of the argument list.
	 *
	 * \param[in] arg The argument to be appended to the list of arguments
	 */
	void append(const std::string &arg);

	/**
	 * \brief Returns TRUE iff no information is contained in this Options
	 * instance.
	 *
	 * \return TRUE if no information is contained in this Options instance
	 */
	bool empty() const;

private:

	/**
	 * \brief Flag to indicate presence of --help option
	 */
	bool help_;

	/**
	 * \brief Flag to indicate presence of --version option
	 */
	bool version_;

	/**
	 * \brief Name of the output stream
	 */
	std::string output_;

	/**
	 * \brief Boolean and valued options
	 */
	std::vector<bool> flags_; // TODO redundant

	/**
	 * \brief Valued options' values
	 */
	std::map<OptionCode, std::string> values_;

	/**
	 * \brief Arguments (non-option values)
	 */
	std::vector<std::string> arguments_;
};

} // namespace arcsapp

#endif

