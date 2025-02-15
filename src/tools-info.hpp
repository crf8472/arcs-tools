#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#define __ARCSTOOLS_TOOLS_INFO_HPP__

/**
 * \file
 *
 * \brief Tools for informational output.
 */

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"               // for StringTable
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
class FileReaderDescriptor;
} // namespace v_1_0_0
} // namespace arcsdec


namespace arcsapp
{
inline namespace v_1_0_0
{
// libarcsdec
using arcsdec::FileReaderDescriptor;

// arcs-tools
using table::StringTable;
using table::StringTableLayout;


/**
 * \brief Compose a result table from descriptors
 */
class InfoResultComposer
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~InfoResultComposer() noexcept;

	/**
	 * \brief Add information represented by a descriptor
	 *
	 * \param[in] descriptor Add the info from this descriptor
	 */
	virtual void add(const FileReaderDescriptor& descriptor)
	= 0;

	/**
	 * \brief Get the collected information
	 *
	 * \return Information collected
	 */
	StringTable table() const;

protected:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows Number of rows
	 * \param[in] cols Number of cols
	 */
	InfoResultComposer(const std::size_t rows, const std::size_t cols);

	std::unique_ptr<StringTableLayout> create_layout() const;

	/**
	 * \brief Format library filenames as separated list.
	 */
	std::string libs_to_sep_list(const FileReaderDescriptor& descriptor,
			const std::string& sep);

	/**
	 * \brief Get the collected information
	 *
	 * \return Information collected
	 */
	StringTable& to_table();

private:

	/**
	 * \brief The collected information
	 */
	StringTable table_;
};

/**
 * \brief Compose list of audio formats.
 */
struct AudioFormatComposer final : public InfoResultComposer
{
	AudioFormatComposer();

	void add(const FileReaderDescriptor& descriptor) final;
};

/**
 * \brief Compose list of ToC formats.
 */
struct ToCFormatComposer final : public InfoResultComposer
{
	ToCFormatComposer();

	void add(const FileReaderDescriptor& descriptor) final;
};

/**
 * \brief Lists of available FileReaders.
 */
class AvailableFileReaders final
{
public:

	/**
	 * \brief List of supported audio readers.
	 *
	 * \return List of supported audio readers.
	 */
	static const StringTable& audio();

	/**
	 * \brief List of supported toc readers.
	 *
	 * \return List of supported toc readers.
	 */
	static const StringTable& toc();
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

