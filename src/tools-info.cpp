/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-info.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"
#endif

#include <algorithm>           // for for_each
#include <functional>          // for functional
#include <iterator>            // for begin, end
#include <memory>              // for make_unique
#include <string>              // for string

#ifndef LIBARCSDEC_DESCRIPTORS_HPP__
#include <arcsdec/descriptor.hpp>   // for FileReaderDescriptor
#endif
#ifndef LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>  // for ARCSCalculator, ToCParser
#endif

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"                // for StringTable
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace info
{

using table::StringTableLayout;


InfoResultComposer::InfoResultComposer(const std::size_t rows,
		const std::size_t cols)
	: table_ { rows, cols }
{
	// empty
}


InfoResultComposer::~InfoResultComposer() noexcept = default;


StringTable InfoResultComposer::table() const
{
	return table_;
}


std::unique_ptr<StringTableLayout> InfoResultComposer::create_layout() const
{
	auto l = std::make_unique<StringTableLayout>();

	l->set_row_header_delims(true);
	l->set_row_inner_delims(true);
	l->set_bottom_delims(true);

	return l;
}


std::string InfoResultComposer::libs_to_sep_list(
		const FileReaderDescriptor& descriptor, const std::string& delim)
{
	// Aggregate all libraries

	std::string deps;
	{
		const auto dependencies { descriptor.libraries() };
		for (const auto& [dep_name, dep_libs] : dependencies)
		{
			deps += dep_libs + delim;
		}
	}

	return deps;
}


StringTable& InfoResultComposer::to_table()
{
	return table_;
}


// AudioFormatComposer


AudioFormatComposer::AudioFormatComposer()
	: InfoResultComposer { 0/* rows */, 4/* cols */ }
{
	using table::Align;

	to_table().set_layout(this->create_layout());

	to_table().set_col_label(0, "ID");

	to_table().set_col_label(1, "Libraries");
	to_table().set_align(1, Align::BLOCK);
	to_table().set_max_width(1, 36);

	to_table().set_col_label(2, "File Formats");
	to_table().set_align(2, Align::BLOCK);
	to_table().set_max_width(2, 13);

	to_table().set_col_label(3, "Codecs");
	to_table().set_align(3, Align::BLOCK);
	to_table().set_max_width(3, 17); // libarcsdec max codec name length
}


void AudioFormatComposer::add(const FileReaderDescriptor& descriptor)
{
	using arcsdec::read::Format;
	using arcsdec::read::Codec;

	auto sep { " " };

	// Space-separated list of Format names

	using format_func = std::string (*)(Format);
	const format_func f = &arcsdec::read::name;
	const std::string fmts = details::to_sep_list(descriptor.formats(), sep, f);

	// Space-separated list of Codec names

	using codec_func = std::string (*)(Codec);
	const codec_func c = &arcsdec::read::name;
	const std::string cdecs = details::to_sep_list(descriptor.codecs(), sep, c);

	// Compose table: Add rows for the current descriptor

	const auto row { to_table().rows() };

	to_table().cell(row, 0) = descriptor.id();
	to_table().cell(row, 1) = libs_to_sep_list(descriptor, sep);
	to_table().cell(row, 2) = fmts;
	to_table().cell(row, 3) = cdecs;
}


// ToCFormatComposer


ToCFormatComposer::ToCFormatComposer()
	: InfoResultComposer { 0/* rows */, 3/* cols */ }
{
	using table::Align;

	to_table().set_layout(this->create_layout());

	to_table().set_col_label(0, "ID");

	to_table().set_col_label(1, "Libraries");
	to_table().set_align(1, Align::BLOCK);
	to_table().set_max_width(1, 36);

	to_table().set_col_label(2, "File Formats");
	to_table().set_align(2, Align::BLOCK);
	to_table().set_max_width(2, 13);
}


void ToCFormatComposer::add(const FileReaderDescriptor& descriptor)
{
	using arcsdec::read::Format;
	using arcsdec::read::Codec;

	auto sep { " " };

	// Space-separated list of Format names

	using format_func = std::string (*)(Format);
	const format_func f = &arcsdec::read::name;
	const std::string fmts = details::to_sep_list(descriptor.formats(), sep, f);

	// Space-separated list of Codec names

	using codec_func = std::string (*)(Codec);
	const codec_func c = &arcsdec::read::name;
	const std::string cdecs = details::to_sep_list(descriptor.codecs(), sep, c);

	// Compose table: Add rows for the current descriptor

	const auto row { to_table().rows() };

	to_table().cell(row, 0) = descriptor.id();
	to_table().cell(row, 1) = libs_to_sep_list(descriptor, sep);
	to_table().cell(row, 2) = fmts;
}


// DefaultReaders


template <class Calculator>
StringTable DefaultReaders(const std::function<
		bool(const FileReaderDescriptor&)>& filter_func,
		InfoResultComposer& builder)
{
	const auto readers { std::make_unique<Calculator>()->readers() };

	//auto builder = std::unique_ptr<InfoResultComposer>();

	const auto collect = [&builder,&filter_func](const auto& key_value_pair)
	{
		if (filter_func(*key_value_pair.second))
		{
			builder.add(*key_value_pair.second);
		}
	};

	using std::cbegin;
	using std::cend;

	std::for_each(cbegin(*readers), cend(*readers), collect);

	return builder.table();
}


// AvailableFileReaders


const StringTable& AvailableFileReaders::audio()
{
	using arcsdec::calc::ARCSCalculator;
	//using arcsdec::read::FileReaderDescriptor;
	using arcsdec::read::InputType;

	AudioFormatComposer c{};

	static StringTable table { DefaultReaders<ARCSCalculator>(
			[](const FileReaderDescriptor& d) -> bool
			{
				return InputType::AUDIO == d.input_type();
			}, c) };

	return table;
}


const StringTable& AvailableFileReaders::toc()
{
	using arcsdec::calc::ToCParser;
	//using arcsdec::read::FileReaderDescriptor;
	using arcsdec::read::InputType;

	ToCFormatComposer c{};

	static StringTable table { DefaultReaders<ToCParser>(
			[](const FileReaderDescriptor& d) -> bool
			{
				return InputType::TOC == d.input_type();
			}, c) };

	return table;
}

} // namespace info
} // namespace v_1_0_0
} // namespace arcsapp

