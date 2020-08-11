#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"
#endif

#include <algorithm>           // for transform
#include <cctype>              // for tolower
#include <iterator>            // for ostream_iterator
#include <sstream>             // for ostringstream

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include <arcsdec/descriptors.hpp>
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


template <class T>
void traverse(FormatCollector &collector)
{
	auto apply_func = std::bind(&FormatCollector::add, &collector,
			std::placeholders::_1);

	T t;
	t.selection().traverse_descriptors(apply_func);
}


// info


template <class T>
StringTable formats()
{
	FormatCollector collector;
	traverse<T>(collector);
	return collector.info();
}


// SupportedFormats


const StringTable& SupportedFormats::audio()
{
	static StringTable table { formats<arcsdec::ARCSCalculator>() };
	return table;
}


const StringTable& SupportedFormats::toc()
{
	static StringTable table { formats<arcsdec::TOCParser>() };
	return table;
}


} //namespace arcsapp

