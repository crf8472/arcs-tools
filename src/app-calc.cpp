#include <arcsdec/descriptor.hpp>
#ifndef __ARCSTOOLS_APPCALC_HPP__
#include "app-calc.hpp"
#endif

#include <algorithm>     // for find, set_intersection
#include <cstdlib>       // for EXIT_SUCCESS
#include <iterator>      // for begin, end, back_inserter
#include <memory>        // for unique_ptr, make_unique
#include <string>        // for string
#include <tuple>         // for get, make_tuple, tuple
#include <utility>       // for move, make_pair, pair
#include <vector>        // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif
#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>      // for ToC, AudioSize
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>    // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>     // for type, Checksum, Checksums,...
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>    // for FileReaderSelection
#endif

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"          // for RegisterApplicationType
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"               // for Options, Configurator
#endif
#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"           // for ARIdLayout
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"           // for ChecksumCalculator
#endif
#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"           // for AvailableFileReaders
#endif
//#ifndef __ARCSTOOLS_TOOLS_TABLE_HPP__
//#include "tools-table.hpp"          // for StringTableLayout,
//									// TableComposer
//#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"               // for ResultObject, Result
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

namespace registered
{
// Enable ApplicationFactory::lookup() to find this application by its name
const auto calc = RegisterApplicationType<ARCalcApplication>("calc");
}

// libarcstk
using arcstk::ARId;
using arcstk::Checksums;
using arcstk::ToC;

// arcsapp
using arid::ARIdLayout;
using arid::ARIdTableLayout;
using arid::RichARId;
using calc::HexLayout;
using table::ATTR;
using table::AddField;
using table::TableComposer;
using table::ColTableComposerBuilder;
using table::FieldCreator;
using table::RowTableComposerBuilder;
using table::TableComposerBuilder;
using table::StringTableLayout;


// CALCBASE

constexpr OptionCode CALCBASE::METAFILE;
constexpr OptionCode CALCBASE::NOTRACKS;
constexpr OptionCode CALCBASE::NOFILENAMES;
constexpr OptionCode CALCBASE::NOOFFSETS;
constexpr OptionCode CALCBASE::NOLENGTHS;
constexpr OptionCode CALCBASE::NOLABELS;
constexpr OptionCode CALCBASE::COLDELIM;
constexpr OptionCode CALCBASE::PRINTID;
constexpr OptionCode CALCBASE::PRINTURL;

constexpr OptionCode CALCBASE::SUBCLASS_BASE;


// CALC

constexpr OptionCode CALC::FIRST;
constexpr OptionCode CALC::LAST;
constexpr OptionCode CALC::ALBUM;
constexpr OptionCode CALC::NOV1;
constexpr OptionCode CALC::NOV2;
constexpr OptionCode CALC::SUMSONLY;
constexpr OptionCode CALC::TRACKSASCOLS;


// ARCalcConfiguratorBase


std::unique_ptr<Options> ARCalcConfiguratorBase::configure_calcbase_options(
		std::unique_ptr<Options> options) const
{
	// Warn About Overridden Info Options

	if (options->is_set(CALCBASE::METAFILE) || not options->no_arguments())
	{
		ARCS_LOG_INFO << "Calculation task requested";

		if (options->is_set(CALCBASE::LIST_TOC_FORMATS))
		{
			ARCS_LOG_WARNING <<
				"Option LIST_TOC_FORMATS is ignored due to calculation task";
			options->unset(CALCBASE::LIST_TOC_FORMATS);
		}

		if (options->is_set(CALCBASE::LIST_AUDIO_FORMATS))
		{
			ARCS_LOG_WARNING <<
				"Option LIST_AUDIO_FORMATS is ignored due to calculation task";
			options->unset(CALCBASE::LIST_AUDIO_FORMATS);
		}
	}

	// Metafile: Get Path + Activate Album Mode

	if (options->is_set(CALCBASE::METAFILE))
	{
		if (options->value(CALCBASE::METAFILE).empty())
		{
			// No Metadata File Specified

			if (options->no_arguments())
			{
				ARCS_LOG_ERROR << "No metafile and no audiofile specified";
				return std::make_unique<Options>();
			}
		}
	}

	return options;
}


// ARCalcConfigurator


void ARCalcConfigurator::do_flush_local_options(OptionRegistry& r) const
{
	using std::end;
	r.insert(end(r),
	{
		// from FORMATBASE

		{ CALC::READERID,
		{  "reader", true, "auto",
			"Force use of audio reader with specified id" }},

		{ CALC::PARSERID,
		{  "parser", true, "auto",
			"Force use of toc parser with specified id" }},

		{ CALC::LIST_TOC_FORMATS,
		{  "list-toc-formats", false, "FALSE",
			"List all supported file formats for TOC metadata" }},

		{ CALC::LIST_AUDIO_FORMATS,
		{  "list-audio-formats", false, "FALSE",
			"List all supported audio codec/container formats" }},

		// from CALCBASE

		{ CALC::METAFILE,
		{  'm', "metafile", true, "none", "Specify toc metadata file to use" }},

		{ CALC::NOTRACKS,
		{  "no-track-nos", false, "FALSE", "Do not print track numbers" }},

		{ CALC::NOFILENAMES,
		{  "no-filenames", false, "FALSE", "Do not print the filenames" }},

		{ CALC::NOOFFSETS,
		{  "no-offsets", false, "FALSE", "Do not print track offsets" }},

		{ CALC::NOLENGTHS,
		{  "no-lengths", false, "FALSE", "Do not print track lengths" }},

		{ CALC::NOLABELS,
		{  "no-labels", false, "FALSE", "Do not print column or row labels" }},

		{ CALC::COLDELIM,
		{  "col-delim", true, "ASCII-32", "Specify column delimiter" }},

		{ CALC::PRINTID,
		{  "print-id", false, "FALSE", "Print AccurateRip Id of the album" }},

		{ CALC::PRINTURL,
		{  "print-url", false, "FALSE", "Print AccurateRip URL of the album" }},

		// from CALC

		{ CALC::FIRST,
		{  "first", false, "FALSE", "Treat first audio file as first track" }},

		{ CALC::LAST,
		{  "last", false, "FALSE", "Treat last audio file as last track" }},

		{ CALC::ALBUM,
		{  "album", false, "FALSE", "Abbreviates \"--first --last\"" }},

		{ CALC::NOV1,
		{  "no-v1", false, "FALSE", "Do not provide ARCSv1" }},

		{ CALC::NOV2,
		{  "no-v2", false, "FALSE", "Do not provide ARCSv2" }},

		{ CALC::SUMSONLY,
		{  "print-sums-only", false, "FALSE", "Print only checksums" }},

		{ CALC::TRACKSASCOLS,
		{  "tracks-as-cols", false, "FALSE", "Print tracks as columns" }}
	});
}


std::unique_ptr<Options> ARCalcConfigurator::do_configure_options(
		std::unique_ptr<Options> coptions) const
{
	auto options = this->configure_calcbase_options(std::move(coptions));

	// Determine whether to set ALBUM mode

	if (options->is_set(CALC::METAFILE))
	{
		// Activate Album Mode

		if (options->is_set(CALC::ALBUM))
		{
			ARCS_LOG_INFO <<
				"Option ALBUM is redundant when METAFILE is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option ALBUM due to METAFILE";
			options->set(CALC::ALBUM);
		}

		if (options->is_set(CALC::FIRST))
		{
			ARCS_LOG_INFO <<
				"Option FIRST is redundant when METAFILE is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option FIRST due to METAFILE";
			options->set(CALC::FIRST);
		}

		if (options->is_set(CALC::LAST))
		{
			ARCS_LOG_INFO <<
				"Option LAST is redundant when METAFILE is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option LAST due to METAFILE";
			options->set(CALC::LAST);
		}
	} else
	{
		// No metafile: Album Mode or Not?

		if (options->is_set(CALC::ALBUM))
		{
			if (not options->is_set(CALC::FIRST))
			{
				ARCS_LOG(DEBUG1) << "Activate option FIRST due to ALBUM";

				options->set(CALC::FIRST);
			} else
			{
				ARCS_LOG_INFO <<
					"Option FIRST is redundant when ALBUM is passed";
			}

			if (not options->is_set(CALC::LAST))
			{
				ARCS_LOG(DEBUG1) << "Activate option LAST due to ALBUM";

				options->set(CALC::LAST);
			} else
			{
				ARCS_LOG_INFO <<
					"Option LAST is redundant when ALBUM is passed";
			}
		} else
		{
			if (options->is_set(CALC::FIRST)
				and options->is_set(CALC::LAST))
			{
				ARCS_LOG(DEBUG1) <<
					"Activate option ALBUM due to FIRST and LAST";
				options->set(CALC::ALBUM);
			}
		}
	}

	// Printing options

	if (options->is_set(CALC::SUMSONLY))
	{
		options->set(CALC::NOTRACKS);
		options->set(CALC::NOFILENAMES);
		options->set(CALC::NOOFFSETS);
		options->set(CALC::NOLENGTHS);
		options->set(CALC::NOLABELS); // Multiple Checksum types?
	}

	return options;
}


// CalcTableCreator


void CalcTableCreator::add_result_fields(std::vector<ATTR>& field_list,
		const print_flag_t /*print_flags*/,
		const std::vector<arcstk::checksum::type>& types_to_print) const
{
	using ChecksumType = arcstk::checksum::type;

	for (const auto& t : types_to_print)
	{
		if (ChecksumType::ARCS1 == t)
		{
			field_list.emplace_back(ATTR::CHECKSUM_ARCS1);
		} else
		{
			if (ChecksumType::ARCS2 == t)
			{
				field_list.emplace_back(ATTR::CHECKSUM_ARCS2);
			}
		}
	}
}


void CalcTableCreator::populate_result_creators(
		std::vector<std::unique_ptr<FieldCreator>>& creators,
		const print_flag_t /*print_flags*/,
		const std::vector<ATTR>& field_list,
		const std::vector<arcstk::checksum::type>& /*types*/,
		const Checksums& checksums) const
{
	// do not repeat the find mechanism
	const auto required = [](const std::vector<ATTR>& fields, const ATTR f)
			{
				using std::cbegin;
				using std::cend;
				using std::find;
				return find(cbegin(fields), cend(fields), f) != cend(fields);
			};

	if (required(field_list, ATTR::CHECKSUM_ARCS1))
	{
		creators.emplace_back(
			std::make_unique<AddField<ATTR::CHECKSUM_ARCS1>>(
				&checksums, this->checksum_layout()));
	}

	if (required(field_list, ATTR::CHECKSUM_ARCS2))
	{
		creators.emplace_back(
			std::make_unique<AddField<ATTR::CHECKSUM_ARCS2>>(
				&checksums, this->checksum_layout()));
	}
}


void CalcTableCreator::assertions(InputTuple t) const
{
	const auto checksums = std::get<1>(t);
	const auto arid      = std::get<2>(t);
	const auto toc       = std::get<3>(t);
	const auto filenames = std::get<4>(t);

	using calc::validate;
	validate(checksums, toc, arid, filenames);
}


void CalcTableCreator::do_init_composer(TableComposer& /*c*/) const
{
	// do nothing
}


std::unique_ptr<Result> CalcTableCreator::do_format(InputTuple t) const
{
	const auto types_to_print = std::get<0>(t);
	const auto checksums  = std::get<1>(t);
	const auto arid       = std::get<2>(t);
	const auto toc        = std::get<3>(t);
	const auto filenames  = std::get<4>(t);
	const auto alt_prefix = std::get<5>(t);

	using arid::build_id;
	using arid::default_arid_layout;

	auto buf = ResultBuffer {};

	if (!arid.empty())
	{
		auto layout { arid_layout()
			? arid_layout()->clone()
			: default_arid_layout(formats_labels()) };

		buf.append(build_id(toc, arid, alt_prefix, *layout));
	}

	const auto print_flags { create_field_requests(toc, filenames) };

	// Create ordered list of table columns

	auto field_list { create_field_types(print_flags) };

	add_result_fields(field_list, print_flags, types_to_print);

	// Populate table with data

	auto creators { std::vector<std::unique_ptr<FieldCreator>>{} };

	populate_creators_list(creators, field_list, filenames, *toc, checksums);

	populate_result_creators(creators, print_flags, field_list, types_to_print,
			checksums);

	// Add table to result

	buf.append(format_table(
				field_list, checksums.size(), formats_labels(), creators));

	return buf.flush();
}


// ARCalcApplicationBase


bool ARCalcApplicationBase::do_calculation_requested(
		const Configuration& config) const
{
	return config.is_set(CALCBASE::METAFILE) || not config.no_arguments();
}


std::vector<arcstk::checksum::type> ARCalcApplicationBase::do_requested_types(
		const Configuration& config) const
{
	// Select the checksum::type(s) to print

	std::vector<arcstk::checksum::type> types = {};

	if (!config.is_set(CALC::NOV1))
	{
		types.push_back(arcstk::checksum::type::ARCS1);
	}
	if (!config.is_set(CALC::NOV2))
	{
		types.push_back(arcstk::checksum::type::ARCS2);
	}

	return types;
}


int ARCalcApplicationBase::do_run(const Configuration& config)
{
	// Is an actual calculation requested?
	if (this->calculation_requested(config))
	{
		auto [ exit_code, result ] = this->run_calculation(config);

		this->output(std::move(result));
		return exit_code;
	}

	// If only info options are present, handle info request

	if (config.is_set(CALC::LIST_TOC_FORMATS))
	{
		Output::instance().output(AvailableFileReaders::toc());
	}

	if (config.is_set(CALC::LIST_AUDIO_FORMATS))
	{
		Output::instance().output(AvailableFileReaders::audio());
	}

	return EXIT_SUCCESS;
}


bool ARCalcApplicationBase::calculation_requested(const Configuration& config)
	const
{
	return do_calculation_requested(config);
}


std::vector<arcstk::checksum::type> ARCalcApplicationBase::requested_types(
			const Configuration& config) const
{
	return do_requested_types(config);
}


std::pair<int, std::unique_ptr<Result>> ARCalcApplicationBase::run_calculation(
			const Configuration& config) const
{
	return do_run_calculation(config);
}


std::unique_ptr<arcsdec::FileReaderSelection>
	ARCalcApplicationBase::create_selection(const OptionCode& request,
		const Configuration& config) const
{
	if (config.is_set(request))
	{
		using arcsdec::InputFormatException;

		auto selection =
			std::unique_ptr<arcsdec::FileReaderSelection>{ nullptr };

		const calc::IdSelection id_selection;

		try
		{
			selection = id_selection(config.value(request));
			return selection;

		} catch (const InputFormatException& e)
		{
			Output::instance().output(std::string
				{ "Failed to acquire file reader '" }
				+ config.value(request)
				+ "'"
			);

			return nullptr;
		}
	}

	return std::unique_ptr<arcsdec::FileReaderSelection>{ nullptr };
}


// ARCalcApplication


std::tuple<Checksums, ARId, std::unique_ptr<ToC>> ARCalcApplication::calculate(
	const std::vector<std::string>& audiofilenames,
	const std::string& metafilename,
	const bool first_file_is_first_track,
	const bool last_file_is_last_track,
	const std::vector<arcstk::checksum::type>& types_requested,
	arcsdec::FileReaderSelection* audio_selection,
	arcsdec::FileReaderSelection* toc_selection)
{
	// The types to calculate are allowed to differ from the explicitly
	// requested types (since e.g. ARCS1 is a byproduct of ARCS2 and the
	// type-to-calculate ARCS2 hence represents both the type-requested ARCS1
	// as well as the type-requested ARCS2).

	auto types_to_calculate { calc::ChecksumTypeset{} };
	for (auto& t : types_requested)
	{
		types_to_calculate.insert(t);
	}

	calc::ChecksumCalculator c { types_to_calculate };
	if (toc_selection)   { c.set_toc_selection  (toc_selection);   }
	if (audio_selection) { c.set_audio_selection(audio_selection); }

	auto [ checksums, arid, toc ] = metafilename.empty()
		? c.calculate(audiofilenames,                    //Tracks/Album w/o ToC
				first_file_is_first_track, last_file_is_last_track)
		: c.calculate(audiofilenames, metafilename);     //Album: w ToC

	return std::make_tuple(checksums, arid, std::move(toc));
}


std::unique_ptr<CalcTableCreator> ARCalcApplication::create_formatter(
		const Configuration& config) const
{
	ARCS_LOG(DEBUG3) << "Define output attributes:";

	auto fmt = std::unique_ptr<CalcTableCreator>
	{
		std::make_unique<CalcTableCreator>()
	};

	// Layouts for Checksums + ARId

	fmt->set_checksum_layout(std::make_unique<HexLayout>());

	// Layout for ARId

	if (config.is_set(CALC::PRINTID) || config.is_set(CALC::PRINTURL))
	{
		std::unique_ptr<ARIdLayout> id_layout =
			std::make_unique<ARIdTableLayout>(
				!config.is_set(CALC::NOLABELS),
				config.is_set(CALC::PRINTID),
				config.is_set(CALC::PRINTURL),
				false, /* no filenames */
				false, /* no tracks */
				false, /* no id 1 */
				false, /* no id 2 */
				false  /* no cddb id */
		);

		fmt->set_arid_layout(std::move(id_layout));
	}

	// ToC present? Helper for determining other properties
	const bool has_toc = !config.value(CALC::METAFILE).empty();

	// Tracks in order?
	const bool tracks_numbered = config.is_set(CALC::FIRST)
		|| config.is_set(CALC::LAST) || has_toc;


	// Print labels or not
	fmt->set_format_labels(!config.is_set(CALC::NOLABELS));

	ARCS_LOG(DEBUG3) << "Print LABEL :   " << fmt->formats_labels();

	// Print track numbers if they are not forbidden and a ToC is present
	fmt->set_format_field(ATTR::TRACK,
			config.is_set(CALC::NOTRACKS) ? false : tracks_numbered);

	ARCS_LOG(DEBUG3) << "Print TRACK :   " << fmt->formats_field(ATTR::TRACK);

	// Print offsets if they are not forbidden and a ToC is present
	fmt->set_format_field(ATTR::OFFSET,
			config.is_set(CALC::NOOFFSETS) ? false : has_toc);

	ARCS_LOG(DEBUG3) << "Print OFFSET:   " << fmt->formats_field(ATTR::OFFSET);

	// Print lengths if they are not forbidden
	fmt->set_format_field(ATTR::LENGTH, !config.is_set(CALC::NOLENGTHS));

	ARCS_LOG(DEBUG3) << "Print LENGTH:   " << fmt->formats_field(ATTR::LENGTH);

	// Print filenames if they are not forbidden and a ToC is _not_ present
	fmt->set_format_field(ATTR::FILENAME,
			config.is_set(CALC::NOFILENAMES) ? false : !has_toc);

	ARCS_LOG(DEBUG3) << "Print FILENAME: " <<
			fmt->formats_field(ATTR::FILENAME);

	auto layout { std::make_unique<StringTableLayout>() };

	// Define delimiters and switch them on or off

	layout->set_col_inner_delim(config.is_set(CALC::COLDELIM)
		? config.value(CALC::COLDELIM)
		: " ");

	// Print tracks either as columns or as rows

	std::unique_ptr<TableComposerBuilder> builder = nullptr;
	if (config.is_set(CALC::TRACKSASCOLS))
	{
		builder = std::make_unique<ColTableComposerBuilder>();

		// delimiter between labels column and column for first track
		layout->set_col_labels_delim(layout->col_inner_delim());
		layout->set_col_labels_delims(true);
	} else
	{
		builder = std::make_unique<RowTableComposerBuilder>();
	}

	fmt->set_table_layout(std::move(layout));
	fmt->set_builder(std::move(builder));

	return fmt;
}


auto ARCalcApplication::do_run_calculation(const Configuration& config) const
	-> std::pair<int, std::unique_ptr<Result>>
{
	// Determine the explicitly requested types

	const auto requested_types = this->requested_types(config);

	if (requested_types.empty()) // No types requested? No calculation required!
	{
		ARCS_LOG_WARNING << "No checksum types requested. Done.";

		return { EXIT_SUCCESS, nullptr };
	}

	// Configure selections (e.g. --reader and --parser)

	auto audio_selection = create_selection(CALC::READERID, config);
	auto toc_selection   = create_selection(CALC::PARSERID, config);

	// If no selections are assigned, the libarcsdec default selections
	// will be used.

	// Perform the actual calculation

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(
			*config.arguments(),
			config.value(CALC::METAFILE),
			config.is_set(CALC::FIRST),
			config.is_set(CALC::LAST),
			requested_types,
			audio_selection.get(),
			toc_selection.get()
	);

	if (checksums.size() == 0)
	{
		fatal_error("Calculation returned no checksums");
	}

	// Types to print = all types requested AND computed

	std::vector<arcstk::checksum::type> types_to_print;
	{
		using std::cbegin;
		using std::cend;

		auto calculated_types { cbegin(checksums)->types() };

		std::set_intersection(
				cbegin(calculated_types), cend(calculated_types),
				cbegin(requested_types),  cend(requested_types),
				std::back_inserter(types_to_print));
	}

	auto result { create_formatter(config)->format(
	/* types  */  types_to_print,
	/* ARCSs  */  checksums,
	/* ARId   */  arid,
	/* ToC    */  toc ? toc.get() : nullptr,
	/* files  */  toc ? toc->filenames() : *config.arguments(),
	/* Prefix */  std::string { /* TODO Implement Alt-Prefix */ }
	)};

	return std::make_pair(EXIT_SUCCESS, std::move(result));
}


std::string ARCalcApplication::do_name() const
{
	return "calc";
}


std::string ARCalcApplication::do_call_syntax() const
{
	return "[OPTIONS] <filename1>, <filename2>, ...";
}


std::unique_ptr<Configurator> ARCalcApplication::do_create_configurator() const
{
	return std::make_unique<ARCalcConfigurator>();
}

} // namespace v_1_0_0
} // namespace arcsapp

