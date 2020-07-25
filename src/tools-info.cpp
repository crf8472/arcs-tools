#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"
#endif

#include <algorithm>           // for transform
#include <iterator>            // for ostream_iterator
#include <sstream>             // for ostringstream

#include <iostream>

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
	: table_ { 0, 5, true, true }
{
	table_.set_title(0, "Name");
	table_.set_width(0, table_.title(0).length() + 6);
	table_.set_alignment(0, true);

	table_.set_title(1, "Formats");
	table_.set_width(1, table_.title(1).length() + 4);
	table_.set_alignment(1, true);

	table_.set_title(2, "Codecs");
	table_.set_width(2, table_.title(2).length() + 4);
	table_.set_alignment(2, true);

	table_.set_title(3, "Lib name");
	table_.set_width(3, table_.title(3).length() + 5);
	table_.set_alignment(3, true);

	table_.set_title(4, "Runtime object");
	table_.set_width(4, table_.title(4).length());
	table_.set_alignment(4, true);
}


void FormatCollector::add(const arcsdec::FileReaderDescriptor &descriptor)
{
	// comma-separated list of format names

	std::ostringstream format_list;

	if (const auto& formats = descriptor.formats(); not formats.empty())
	{
		if (formats.size() == 1)
		{
			format_list << arcsdec::name(*formats.begin());
		} else
		{
			std::transform(formats.begin(), --formats.rbegin().base(),
				std::ostream_iterator<std::string>(format_list, ","),
				[](const arcsdec::Format &format) -> std::string
				{
					return arcsdec::name(format);
				});

			format_list << arcsdec::name(*formats.rbegin());
		}
	}

	// comma-separated list of codecs

	std::ostringstream codec_list;

	if (const auto& codecs = descriptor.codecs(); not codecs.empty())
	{
		if (codecs.size() == 1)
		{
			codec_list << arcsdec::name(*codecs.begin());
		} else
		{
			std::transform(codecs.begin(), --codecs.rbegin().base(),
				std::ostream_iterator<std::string>(codec_list, ","),
				[](const arcsdec::Codec &codec) -> std::string
				{
					return arcsdec::name(codec);
				});

			codec_list << arcsdec::name(*codecs.rbegin());
		}
	}

	// Add row for the current descriptor

	int row = table_.current_row();

	table_.update_cell(row, 0, descriptor.name());
	table_.update_cell(row, 1, format_list.str());
	table_.update_cell(row, 2, codec_list.str());

	auto dependencies = descriptor.libraries();

	for (const auto& dep : dependencies)
	{
		table_.update_cell(row, 3, dep.first);
		table_.update_cell(row, 4, dep.second);
		++row;
	}
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

