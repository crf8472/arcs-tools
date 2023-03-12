#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"
#endif

#include <algorithm>           // for transform
#include <cctype>              // for tolower
#include <iterator>            // for ostream_iterator
#include <sstream>             // for ostringstream

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include <arcsdec/descriptor.hpp>   // for FileReaderDescriptor
#endif
#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>  // for ARCSCalculator, TOCParser
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"          // for StringTable
#endif

namespace arcsapp
{

/**
 * \brief Collect descriptor infos
 */
class FormatCollector final
{
public:

	/**
	 * \brief Constructor
	 */
	FormatCollector();

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
	const StringTable& info() const;

	/**
	 * \brief TRUE while nothing was collected.
	 *
	 * \return TRUE while nothign was collected, otherwise FALSE.
	 */
	bool empty() const;

private:

	/**
	 * \brief The collected information
	 */
	StringTable table_;
};


FormatCollector::FormatCollector()
	: table_ { 0/* rows */, 2/* cols */, true/* append rows */ }
{
	table_.set_title(0, "Name");
	table_.set_dynamic_width(0);
	table_.set_alignment(0, true);

	table_.set_title(1, "Properties");
	table_.set_dynamic_width(1);
	table_.set_alignment(1, true);
}


void FormatCollector::add(const arcsdec::FileReaderDescriptor &descriptor)
{
	// aggregate of all involved libraries

	auto dependencies = descriptor.libraries();

	// comma-separated list of format names
	// transform container formats to filetype suffices

	using format_func = std::string (*)(arcsdec::Format);

	format_func f = [](arcsdec::Format f) -> std::string
	{
		auto name = arcsdec::name(f);
		std::transform(name.begin(), name.end(), name.begin(),
			[](unsigned char c){ return std::tolower(c); });
		return "." + name;
	};

	std::string formats = details::to_sep_list(descriptor.formats(), ",", f);

	// comma-separated list of codec names

	using codec_func = std::string (*)(arcsdec::Codec);
	codec_func c = &arcsdec::name;
	std::string codecs = details::to_sep_list(descriptor.codecs(), ",", c);

	// Compose table: Add row for the current descriptor

	int row = table_.current_row();
	table_.update_cell(row, 0, descriptor.name());
	for (const auto& dep : dependencies)
	{
		table_.update_cell(row++, 1, dep.second); // dep.first == libname
	}
	table_.update_cell(row++, 1, formats);
	table_.update_cell(row++, 1, codecs);
}


const StringTable& FormatCollector::info() const
{
	return table_;
}


bool FormatCollector::empty() const
{
	return table_.empty();
}


// traverse


template <class Container>
void traverse(FormatCollector &collector, const Container& c)
{
	const auto cp = &collector;
	std::for_each(c.readers()->begin(), c.readers()->end(),
		[cp](const auto& key_value_pair)
		{
			cp->add(*key_value_pair.second);
		});
}


// info


template <class Calculator>
StringTable DefaultReaders()
{
	FormatCollector collector;
	{
		Calculator calculator;
		traverse<Calculator>(collector, calculator);
	}
	return collector.info();
}


// AvailableFileReaders


const StringTable& AvailableFileReaders::audio()
{
	static StringTable table { DefaultReaders<arcsdec::ARCSCalculator>() };
	return table;
}


const StringTable& AvailableFileReaders::toc()
{
	static StringTable table { DefaultReaders<arcsdec::TOCParser>() };
	return table;
}


} //namespace arcsapp

