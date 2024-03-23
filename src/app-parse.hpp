#ifndef __ARCSTOOLS_APPPARSE_HPP__
#define __ARCSTOOLS_APPPARSE_HPP__

/**
 * \file
 *
 * \brief Interface for ARParseApplication.
 *
 * Options, Configurator and Application for parse.
 */


#include <memory>           // for unique_ptr
#include <string>           // for string


#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"     // for Application
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

