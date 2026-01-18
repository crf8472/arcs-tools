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

// libarcstk
using arcstk::DBAR;
using arcstk::ParseHandler;
using arcstk::ParseErrorHandler;

// arcsapp
using arcsapp::arid::ARIdLayout;


/**
 * \brief Interface for formatting DBARTriplets.
 */
using DBARTripletLayout = Layout<std::string, unsigned, arcstk::DBARTriplet>;


/**
 * \brief Output format for DBARs.
 *
 * This format is intended for "direct" output via PrintParseHandler. Each
 * entity is formatted and printed as passed without aggregation or caching.
 *
 * \see PrintParseHandler
 */
class DBAROutputFormat
{
	virtual std::string do_start_input() const = 0;

	virtual std::string do_start_block() const = 0;

	virtual std::string do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const;

	virtual std::string do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const;

	virtual std::string do_end_block() const = 0;

	virtual std::string do_end_input() const = 0;

	/**
	 * \brief Internal block counter.
	 */
	mutable unsigned block_counter_;

	/**
	 * \brief Internal track counter.
	 */
	mutable unsigned track_counter_;

	/**
	 * \brief Internal layout used for printing the ARIds.
	 */
	std::unique_ptr<ARIdLayout> arid_layout_;

	/**
	 * \brief Internal layout used for printing the triplets.
	 */
	std::unique_ptr<DBARTripletLayout> triplet_layout_;

protected:

	/**
	 * \brief Create ostringstream for output.
	 */
	std::ostringstream create_stream() const;

	/**
	 * \brief Non-const-access to the print layout for subclasses.
	 *
	 * \return The print layout used
	 */
	ARIdLayout* arid_layout_ptr() const;

	/**
	 * \brief Non-const-access to the print layout for subclasses.
	 *
	 * \return The print layout used
	 */
	DBARTripletLayout* triplet_layout_ptr() const;

public:

	/**
	 * \brief Default constructor.
	 */
	DBAROutputFormat();

	/**
	 * \brief Constructor with layouts.
	 *
	 * \param[in] arid_layout    Layout for ARIds
	 * \param[in] triplet_layout Layout for DBAR triplets
	 */
	DBAROutputFormat(std::unique_ptr<ARIdLayout> arid_layout,
			std::unique_ptr<DBARTripletLayout> triplet_layout);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DBAROutputFormat() noexcept = default;

	/**
	 * \brief String to print on start_input.
	 *
	 * \return String to print on start_input
	 */
	std::string start_input() const;

	/**
	 * \brief String to print on start_block.
	 *
	 * \return String to print on start_block
	 */
	std::string start_block() const;

	/**
	 * \brief Print block header.
	 *
	 * \param[in] track_count Total number of tracks
	 * \param[in] id1         Disc id1
	 * \param[in] id2         Disc id2
	 * \param[in] cddb_id     CDDB id
	 */
	std::string header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const;

	/**
	 * \brief Print triplet.
	 *
	 * \param[in] arcs          ARCS value
	 * \param[in] confidence    Confidence
	 * \param[in] frame450_arcs ARCS value for frame 450
	 */
	std::string triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const;

	/**
	 * \brief String to print on end_block.
	 *
	 * \return String to print on end_block
	 */
	std::string end_block() const;

	/**
	 * \brief String to print on end_input.
	 *
	 * \return String to print on end_input
	 */
	std::string end_input() const;

	/**
	 * \brief Current block counter value.
	 */
	unsigned block_counter() const;

	/**
	 * \brief Current track counter value.
	 */
	unsigned track_counter() const;

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
};


/**
 * \brief Implements 'text_decorated' for triplets.
 */
class TextDecoratedTripletLayout final : public DBARTripletLayout
{
public:

	using DBARTripletLayout::Layout;

	/**
	 * \brief Default constructor.
	 */
	TextDecoratedTripletLayout();

	/**
	 * \brief Set printing width for ARCS value in chars.
	 *
	 * \param[in] width Number of chars
	 */
	void set_width_arcs(const int width);

	/**
	 * \brief Return printing width for ARCS values.
	 *
	 * \return Number of chars for printed ARCS values
	 */
	int width_arcs() const;

	/**
	 * \brief Set printing width for confidence value in chars.
	 *
	 * \param[in] width Number of chars
	 */
	void set_width_conf(const int width);

	/**
	 * \brief Return printing width for confidence values.
	 *
	 * \return Number of chars for printed confidence values
	 */
	int width_conf() const;

	/**
	 * \brief Set symbol to print for unparsed values.
	 *
	 * \param[in] s Symbol for unparsed values
	 */
	void set_unparsed_value_symbol(const std::string& s);

	/**
	 * \brief Return symbol for unparsed values.
	 *
	 * \return Symbol for unparsed values.
	 */
	std::string set_unparsed_value_symbol() const;

	/**
	 * \brief Activate label.
	 *
	 * \param[in] flag TRUE activates label printing
	 */
	void set_with_label(const bool flag);

	/**
	 * \brief Return whether label printing is activated.
	 *
	 * \return TRUE iff label printing is activated, otherwise FALSE.
	 */
	bool with_label() const;

private:

	/**
	 * \brief Internal width for ARCS.
	 */
	int width_arcs_;

	/**
	 * \brief Internal width for confidence.
	 */
	int width_conf_;

	/**
	 * \brief Internal symbol for unparsed values.
	 */
	std::string unparsed_value_;

	/**
	 * \brief Internal flag for printing labels.
	 */
	bool with_label_;

	// TODO checksum_layout_

	// no assertions()

	std::string do_format(InputTuple t) const override;
};


/**
 * \brief Implements format 'text_decorated'.
 */
class TextDecoratedFormat final : public DBAROutputFormat
{
	std::string do_start_input() const final;

	std::string do_start_block() const final;

	// do_header() from DBAROutputFormat

	// do_triplet() from DBAROutputFormat

	std::string do_end_block() const final;

	std::string do_end_input() const final;
};


/**
 * \brief Implements 'yaml' for triplets.
 */
class YamlTripletLayout final : public DBARTripletLayout
{
	// no assertions()

	std::string do_format(InputTuple t) const override;

public:

	using DBARTripletLayout::Layout;
};


/**
 * \brief Implements format 'yaml'.
 */
class YamlFormat final : public DBAROutputFormat
{
	std::string do_start_input() const final;

	std::string do_start_block() const final;

	std::string do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const final;

	// do_triplet() from DBAROutputFormat

	std::string do_end_block() const final;

	std::string do_end_input() const final;

public:

	/**
	 * \brief Default constructor.
	 */
	YamlFormat();
};


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
	void set_format(std::unique_ptr<DBAROutputFormat> format);

	/**
	 * \brief Read-access to the print layout used for ARIds.
	 *
	 * \return The print layout used for ARIds
	 */
	const DBAROutputFormat& format() const;

	/**
	 * \brief Specify a file as print target.
	 *
	 * \param[in] filename Name of the out file
	 */
	void set_outfile(const std::string& filename);

protected:

	/**
	 * \brief Non-const-access to the output format for subclasses.
	 *
	 * \return The output format used
	 */
	DBAROutputFormat* format();

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
	 * \brief Internal output format.
	 */
	std::unique_ptr<DBAROutputFormat> format_;
};

} // namespace dbar
} // namespace v_1_0_0
} // namespace arcsapp

#endif

