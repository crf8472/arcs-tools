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
	: table_ { 0, 4, true, true }
{
	table_.set_title(0, "Name");
	table_.set_width(0, table_.title(0).length() + 6);
	table_.set_alignment(0, true);

	table_.set_title(1, "Description");
	table_.set_width(1, table_.title(1).length() + 4);
	table_.set_alignment(1, true);

	table_.set_title(2, "Lib name");
	table_.set_width(2, table_.title(2).length() + 5);
	table_.set_alignment(2, true);

	table_.set_title(3, "Runtime object");
	table_.set_width(3, table_.title(3).length());
	table_.set_alignment(3, true);
}


void FormatCollector::add(const arcsdec::FileReaderDescriptor &descriptor)
{
	// Compose description: comma-separated list of format names

	std::ostringstream desc;

	if (const auto& formats = descriptor.formats(); not formats.empty())
	{
		if (formats.size() == 1)
		{
			desc << arcsdec::name(*formats.begin());
		} else
		{
			// FIXME Always ends with a comma
			std::transform(formats.begin(), formats.rbegin().base(),
				std::ostream_iterator<std::string>(desc, ","),
				[](const arcsdec::FileFormat &format) -> std::string
				{
					return arcsdec::name(format);
				});

			desc << arcsdec::name(*formats.rbegin());
		}
	}

	// Add row for the current descriptor

	int row = table_.current_row();

	table_.update_cell(row, 0, descriptor.name());
	table_.update_cell(row, 1, desc.str());

	auto dependencies = descriptor.libraries();

	for (const auto& dep : dependencies)
	{
		table_.update_cell(row, 2, dep.first);
		table_.update_cell(row, 3, dep.second);
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

