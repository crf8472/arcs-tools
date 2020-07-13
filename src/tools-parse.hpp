#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#define __ARCSTOOLS_TOOLS_PARSE_HPP__

/**
 * \file
 *
 * \brief Content and error handlers for ARParsers.
 *
 * Provides parser handlers for ARParsers.
 */

#include <cstdint>           // for uint32_t, uint8_t
#include <fstream>           // for ofstream, ostream
#include <memory>            // for unique_ptr
#include <string>            // for string

#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif

namespace arcsapp
{

class ARIdLayout;
class ARTripletLayout;

using arcstk::ContentHandler;

/**
 * \brief Content handler that just prints the parsed content immediately.
 *
 * Printing is performed to stdout by default. If a filename is specified,
 * printing goes to this file instead.
 */
class ARParserContentPrintHandler : public ContentHandler
{

public:

	/**
	 * \brief Constructor for printing to a file.
	 *
	 * \param[in] filename Name of the out file
	 */
	ARParserContentPrintHandler(const std::string &filename);

	/**
	 * \brief Constructor.
	 */
	ARParserContentPrintHandler() : ARParserContentPrintHandler("")
	{ /* empty */ };

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARParserContentPrintHandler() noexcept;

	/**
	 * \brief Sets the layout for printing ARIds.
	 *
	 * \param[in] layout The print layout to use
	 */
	void set_arid_layout(std::unique_ptr<ARIdLayout> layout);

	/**
	 * \brief Read-access to the print layout used for ARIds.
	 *
	 * \return The print layout used for ARIds
	 */
	const ARIdLayout& arid_layout() const;

	/**
	 * \brief Sets the layout for printing track information.
	 *
	 * \param[in] layout The print layout to use
	 */
	void set_triplet_layout(std::unique_ptr<ARTripletLayout> layout);

	/**
	 * \brief Read-access to the print layout used for track information.
	 *
	 * \return The print layout used for track information
	 */
	const ARTripletLayout& triplet_layout() const;

	/**
	 * \brief Specify a file as print target.
	 *
	 * \param[in] filename Name of the out file
	 */
	void set_outfile(const std::string &filename);

protected:

	/**
	 * \brief Non-const-access to the print layout for subclasses.
	 *
	 * \return The print layout used
	 */
	ARIdLayout* arid_layout();

	/**
	 * \brief Non-const-access to the print layout for subclasses.
	 *
	 * \return The print layout used
	 */
	ARTripletLayout* triplet_layout();

private:

	void do_start_input() override;

	void do_start_block() override;

	void do_id(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) override;

	void do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) override;

	void do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs,
			const bool arcs_valid,
			const bool confidence_valid,
			const bool frame450_arcs_valid) override;

	void do_end_block() override;

	void do_end_input() override;

	/**
	 * \brief Internal block counter.
	 */
	uint32_t block_counter_;

	/**
	 * \brief Internal track counter.
	 */
	int track_;

	/**
	 * \brief Internal layout used for printing the ARIds.
	 */
	std::unique_ptr<ARIdLayout> arid_layout_;

	/**
	 * \brief Internal layout used for printing the triplets.
	 */
	std::unique_ptr<ARTripletLayout> triplet_layout_;

	/**
	 * \brief Internal print file stream.
	 */
	std::ofstream out_file_stream_;

	/**
	 * \brief Internal print target stream.
	 */
	std::ostream out_stream_;
};

} // namespace arcsapp

#endif

