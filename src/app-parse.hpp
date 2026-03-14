#ifndef ARCSTOOLS_APPPARSE_HPP_
#define ARCSTOOLS_APPPARSE_HPP_
/**
 * \file
 *
 * \brief Interface for ARParseApplication.
 *
 * Options, Configurator and Application for parse.
 */


#include <memory>           // for unique_ptr
#include <string>           // for string

#ifndef ARCSTOOLS_APPLICATION_HPP_
#include "application.hpp"  // for Application
#endif
#ifndef ARCSTOOLS_CONFIG_HPP_
#include "config.hpp"       // for Configurator, OptionCode
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{


class Application;
class Configurator;
class Options;
class Result;


/**
 * \brief Configuration options for ARParseApplications.
 */
class ARParseOptions final : private OPTION
{
	//static constexpr OptionCode BASE      = 0;

	static constexpr auto& BASE = OPTION::SUBCLASS_BASE;

public:

	static constexpr OptionCode FORMAT    = BASE +  0; // 7
	static constexpr OptionCode F_DETAILS = BASE +  1;
};


/**
 * \brief Configurator for ARParseApplication instances.
 */
class ARParseConfigurator final : public Configurator
{
public:

	using Configurator::Configurator;

private:

	void do_flush_local_options(OptionRegistry& r) const final;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) const final;

	// void do_validate(const Options& options) const;

	// OptionParsers do_parser_list() const;

	// void do_validate(const Configuration& configuration) const;
};


/**
 * \brief Application to parse AccurateRip responses.
 */
class ARParseApplication final : public Application
{
	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> do_create_configurator() const final;

	int do_run(const Configuration& config) final;
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

