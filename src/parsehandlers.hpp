#ifndef __ARCSTOOLS_PARSEHANDLERS_HPP__
#define __ARCSTOOLS_PARSEHANDLERS_HPP__

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

class ARIdLayout;
class ARTripletFormat;

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
	 * \brief Constructor.
	 */
	ARParserContentPrintHandler();

	/**
	 * \brief Constructor for printing to a file.
	 *
	 * \param[in] filename Name of the out file
	 */
	ARParserContentPrintHandler(const std::string &filename);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARParserContentPrintHandler() noexcept;

	/**
	 * \brief Sets the format for printing ARIds.
	 *
	 * \param[in] format The print format to use
	 */
	void set_arid_format(std::unique_ptr<ARIdLayout> format);

	/**
	 * \brief Read-access to the print format used for ARIds.
	 *
	 * \return The print format used for ARIds
	 */
	const ARIdLayout& arid_format() const;

	/**
	 * \brief Sets the format for printing track information.
	 *
	 * \param[in] format The print format to use
	 */
	void set_triplet_format(std::unique_ptr<ARTripletFormat> format);

	/**
	 * \brief Read-access to the print format used for track information.
	 *
	 * \return The print format used for track information
	 */
	const ARTripletFormat& triplet_format() const;

	/**
	 * \brief Specify a file as print target.
	 *
	 * \param[in] filename Name of the out file
	 */
	void set_outfile(const std::string &filename);


protected:

	/**
	 * \brief Non-const-access to the print format for subclasses.
	 *
	 * \return The print format used
	 */
	ARIdLayout* arid_layout();

	/**
	 * \brief Non-const-access to the print format for subclasses.
	 *
	 * \return The print format used
	 */
	ARTripletFormat* triplet_format();


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
	uint8_t track_;

	/**
	 * \brief Internal format used for printing the ARIds.
	 */
	std::unique_ptr<ARIdLayout> arid_format_;

	/**
	 * \brief Internal format used for printing the triplets.
	 */
	std::unique_ptr<ARTripletFormat> triplet_format_;

	/**
	 * \brief Internal print file stream.
	 */
	std::ofstream out_file_stream_;

	/**
	 * \brief Internal print target stream.
	 */
	std::ostream out_stream_;
};

#endif

