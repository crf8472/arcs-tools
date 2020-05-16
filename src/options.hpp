#ifndef __ARCSTOOLS_OPTIONS_HPP__
#define __ARCSTOOLS_OPTIONS_HPP__

/**
 * \file
 *
 * \brief Options for runtime configuration.
 *
 * The Options class represents the complete command line input to an
 * application. Options are produced by a Configurator internally using a
 * CLIParser.
 */

#include <cstdint>   // for uint16_t
#include <map>       // for map
#include <string>    // for string
#include <vector>    // for vector


/**
 * \brief Base class for options and arguments. Represents the entire command
 * line input.
 */
class Options
{

public:

	/**
	 * \brief Default constructor.
	 */
	Options();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Options() noexcept;

	/**
	 * \brief Set or unset the version flag.
	 *
	 * The version flag indicates whether the option --version was passed.
	 *
	 * \param[in] version The flag to set or unset
	 */
	void set_version(const bool &version);

	/**
	 * \brief Return the version flag.
	 *
	 * \return TRUE iff the version option is set, otherwise FALSE
	 */
	bool is_set_version() const;

	/**
	 * \brief Inherited worker to implement getters for option checks.
	 *
	 * \param[in] option The option to check for
	 *
	 * \return TRUE iff the option is set, otherwise FALSE
	 */
	bool is_set(const uint16_t &option) const;

	/**
	 * \brief Set the option to TRUE.
	 *
	 * If the option is currently set, the call has no effect.
	 *
	 * \param[in] option The option to be set to TRUE
	 */
	void set(const uint16_t &option);

	/**
	 * \brief Set the option to FALSE.
	 *
	 * If the option is currently unset, the call has no effect.
	 *
	 * \param[in] option The option to be set to FALSE
	 */
	void unset(const uint16_t &option);

	/**
	 * \brief Get option value by key.
	 *
	 * \param[in] option The option whose value to get
	 *
	 * \return The value of the option passed
	 */
	std::string get(const uint16_t &option) const;

	/**
	 * \brief Set option value for key.
	 *
	 * \param[in] option The option to put in
	 * \param[in] value  The value for the option to put in
	 */
	void put(const uint16_t &option, const std::string &value);

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
	std::string const get_argument(const uint16_t &i) const;

	/**
	 * \brief Puts an argument to the end of the argument list.
	 *
	 * \param[in] arg The argument to be appended to the list of arguments
	 */
	void push_back_argument(const std::string &arg);

	/**
	 * \brief Returns TRUE iff no information is contained in this Options
	 * instance.
	 *
	 * \return TRUE if no information is contained in this Options instance
	 */
	bool empty() const;

	/**
	 * Position of the leftmost set flag.
	 *
	 * \return Position of the leftmost flag that is set.
	 */
	uint16_t leftmost_flag() const;

	/**
	 * Position of the rightmost set flag.
	 *
	 * \return Position of the rightmost flag that is set.
	 */
	uint16_t rightmost_flag() const;


private:

	/**
	 * \brief Flag to indicate presence of --version option
	 */
	bool version_;

	/**
	 * \brief Boolean and valued options
	 */
	uint16_t config_;

	/**
	 * \brief Valued options' values
	 */
	std::map<uint16_t, std::string> option_map_;

	/**
	 * \brief Arguments (non-option values)
	 */
	std::vector<std::string> arguments_;

};

#endif

