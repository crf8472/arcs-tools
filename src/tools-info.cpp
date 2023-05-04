#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"
#endif

#include <algorithm>           // for for_each
#include <functional>          // for functional
#include <iterator>            // for begin, end
#include <memory>              // for make_unique
#include <string>              // for string

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include <arcsdec/descriptor.hpp>   // for FileReaderDescriptor
#endif
#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>  // for ARCSCalculator, TOCParser
#endif

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"                // for StringTable
#endif

namespace arcsapp
{


/**
 * \brief Collect descriptor infos
 */
class InfoResultComposer final
{
public:

	/**
	 * \brief Constructor
	 */
	InfoResultComposer();

	/**
	 * \brief Add information represented by a descriptor
	 *
	 * \param[in] descriptor Add the info from this descriptor
	 */
	void add(const arcsdec::FileReaderDescriptor &descriptor);

	/**
	 * \brief Get the collected information
	 *
	 * \return Information collected
	 */
	const StringTable& table() const;

private:

	/**
	 * \brief The collected information
	 */
	StringTable table_;
};


InfoResultComposer::InfoResultComposer()
	: table_ { 0/* rows */, 4/* cols */ }
{
	using table::Align;

	table_.set_col_label(0, "ID");

	table_.set_col_label(1, "Libraries");
	table_.set_align(1, Align::BLOCK);

	table_.set_col_label(2, "File Formats");
	table_.set_align(2, Align::BLOCK);
	table_.set_max_width(2, 13);

	table_.set_col_label(3, "Codecs");
	table_.set_align(3, Align::BLOCK);
	table_.set_max_width(3, 17); // libarcsdec max codec name length
}


void InfoResultComposer::add(const arcsdec::FileReaderDescriptor &descriptor)
{
	const std::string sep { " " };

	// Aggregate all libraries

	std::string deps;
	{
		const auto dependencies = descriptor.libraries();
		for (const auto& [dep_name, dep_libs] : dependencies)
		{
			deps += dep_libs + sep;

			if (table_.max_width(1) < dep_libs.length())
			{
				table_.set_max_width(1, dep_libs.length());
			}
		}
	}

	// Space-separated list of Format names

	using format_func = std::string (*)(arcsdec::Format);
	const format_func f = &arcsdec::name;
	const std::string fmts = details::to_sep_list(descriptor.formats(), sep, f);

	// Space-separated list of Codec names

	using codec_func = std::string (*)(arcsdec::Codec);
	const codec_func c = &arcsdec::name;
	const std::string cdecs = details::to_sep_list(descriptor.codecs(), sep, c);

	// Compose table: Add rows for the current descriptor

	const auto row { table_.rows() };

	table_.cell(row, 0) = descriptor.id();
	table_.cell(row, 1) = deps;
	table_.cell(row, 2) = fmts;
	table_.cell(row, 3) = cdecs;
}


const StringTable& InfoResultComposer::table() const
{
	return table_;
}


// DefaultReaders


template <class Calculator>
StringTable DefaultReaders(const std::function<
		bool(const arcsdec::FileReaderDescriptor &descriptor)>& filter_func)
{
	const auto readers { std::make_unique<Calculator>()->readers() };

	auto builder = InfoResultComposer{};
	const auto collect = [&builder,&filter_func](const auto& key_value_pair)
	{
		if (filter_func(*key_value_pair.second))
		{
			builder.add(*key_value_pair.second);
		}
	};

	using std::begin;
	using std::end;

	std::for_each(begin(*readers), end(*readers), collect);

	return builder.table();
}


// AvailableFileReaders


const StringTable& AvailableFileReaders::audio()
{
	using arcsdec::FileReaderDescriptor;
	using arcsdec::InputType;

	static StringTable table { DefaultReaders<arcsdec::ARCSCalculator>(
			[](const FileReaderDescriptor& d) -> bool
			{
				return InputType::AUDIO == d.input_type();
			}) };

	return table;
}


const StringTable& AvailableFileReaders::toc()
{
	using arcsdec::FileReaderDescriptor;
	using arcsdec::InputType;

	static StringTable table { DefaultReaders<arcsdec::TOCParser>(
			[](const FileReaderDescriptor& d) -> bool
			{
				return InputType::TOC == d.input_type();
			}) };

	return table;
}


} //namespace arcsapp

