#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"
#endif

#include <algorithm>           // for transform
#include <iterator>            // for ostream_iterator
#include <sstream>             // for ostringstream

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>  // for ARCSCalculator, TOCParser
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"          // for StringTable
#endif

namespace arcsapp
{

//using arcsdec::FileReaderDescpriptor;


/**
 * \brief Collect descriptor infos
 */
class FormatCollector
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
	StringTable& info();

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
	: table_ { 0, 4 }
{
	table_.set_title(0, "Name");
	table_.set_width(0, table_.title(0).length() + 6);
	table_.set_alignment(0, true);

	table_.set_title(1, "Short Desc.");
	table_.set_width(1, table_.title(1).length() + 4);
	table_.set_alignment(1, true);

	table_.set_title(2, "Lib");
	table_.set_width(2, table_.title(2).length() + 5);
	table_.set_alignment(2, true);

	table_.set_title(3, "Version");
	table_.set_width(3, table_.title(3).length());
	table_.set_alignment(3, true);
}


void FormatCollector::add(const arcsdec::FileReaderDescriptor &descriptor)
{
	// FIXME Implement this (currently a mess, just a sketch)

	auto name = descriptor.name();

	// Description: for now, just concatenate the format names
	std::ostringstream desc;
	const auto& formats = descriptor.formats();
	if (!formats.empty())
	{
		std::transform(formats.begin(), formats.rbegin().base(),
			std::ostream_iterator<std::string>(desc, ","),
			[](const arcsdec::FileFormat &format) -> std::string
			{
				return arcsdec::name(format);
			});
		desc << arcsdec::name(*formats.rbegin());
	}

	// FIXME Get the name of the library used at runtime from libarcsdec

	// FIXME Get the version of the library used at runtime from libarcsdec

	int row = table_.current_row();

	// Add row
	table_.update_cell(row, 0, name);
	table_.update_cell(row, 1, desc.str());
	table_.update_cell(row, 2, "-");
	table_.update_cell(row, 3, "-");

	// Adjust col width to optimal width after each row
	for (std::size_t col = 0; col < table_.columns(); ++col)
	{
		if (static_cast<std::size_t>(table_.width(col))
			< table_(row, col).length())
		{
			table_.set_width(col, table_(row, col).length());
		}
	}

	++row;
}


StringTable& FormatCollector::info()
{
	return table_;
}


bool FormatCollector::empty() const
{
	return table_.empty();
}


// SupportedFormats


const StringTable& SupportedFormats::audio()
{
	static FormatCollector collector;

	if (collector.empty())
	{
		auto apply_func = std::bind(&FormatCollector::add, &collector,
			std::placeholders::_1);

		arcsdec::ARCSCalculator c;
		c.selection().traverse_descriptors(apply_func);
	}

	return collector.info();
}


const StringTable& SupportedFormats::toc()
{
	static FormatCollector collector;

	if (collector.empty())
	{
		auto apply_func = std::bind(&FormatCollector::add, &collector,
			std::placeholders::_1);

		arcsdec::TOCParser p;
		p.selection().traverse_descriptors(apply_func);
	}

	return collector.info();
}


} //namespace arcsapp

