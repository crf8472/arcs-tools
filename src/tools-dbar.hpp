#ifndef __ARCSTOOLS_TOOLS_DBAR_HPP__
#define __ARCSTOOLS_TOOLS_DBAR_HPP__

/**
 * \file
 *
 * \brief Content and error handlers for DBAR parsers.
 *
 * Provides parser handlers for parsing DBAR files.
 */

#include <cstdint>           // for uint32_t, uint8_t
#include <memory>            // for unique_ptr
#include <string>            // for string

#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>   // for DBAR, ParseHandler, ParseErrorHandler
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"       // for Layout
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

// forward declarations
namespace arid
{
class Application;
class ARIdLayout;
}

/**
 * \brief Tools and helpers for DBAR input.
 */
namespace dbar
{

class DBARTripletLayout;

// libarcstk
using arcstk::DBAR;
using arcstk::ParseHandler;
using arcstk::ParseErrorHandler;

// arcsapp
using arcsapp::arid::ARIdLayout;


/**
 * \brief ParseHandler that just prints the parsed content immediately.
 *
 * Printing is performed to Output by default.
 */
class PrintParseHandler final : public ParseHandler
{
public:

	/**
	 * \brief Constructor for printing to output.
	 */
	PrintParseHandler();

	/**
	 * \brief Virtual default destructor.
	 */
	~PrintParseHandler() noexcept;

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
	void set_triplet_layout(std::unique_ptr<DBARTripletLayout> layout);

	/**
	 * \brief Read-access to the print layout used for track information.
	 *
	 * \return The print layout used for track information
	 */
	const DBARTripletLayout& triplet_layout() const;

	/**
	 * \brief Specify a file as print target.
	 *
	 * \param[in] filename Name of the out file
	 */
	void set_outfile(const std::string& filename);

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
	DBARTripletLayout* triplet_layout();

	/**
	 * \brief Print the string.
	 *
	 * \param[in] str String to print
	 */
	void print(const std::string& str) const;

private:

	void do_start_input() final;

	void do_start_block() final;

	void do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) final;

	void do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) final;

	void do_end_block() final;

	void do_end_input() final;

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
	std::unique_ptr<DBARTripletLayout> triplet_layout_;
};


/**
 * \brief Interface for formatting DBARTriplets.
 */
using TripletLayout = Layout<std::string, int, arcstk::DBARTriplet>;


/**
 * \brief Interface for formatting DBARTriplet instances for output.
 */
class DBARTripletLayout : public TripletLayout
{
public:

	using TripletLayout::Layout;

private:

	// no assertions()

	std::string do_format(InputTuple t) const override;
};

} // namespace dbar
} // namespace v_1_0_0
} // namespace arcsapp

#endif

