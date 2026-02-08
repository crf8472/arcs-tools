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

	virtual std::string do_start_triplets() const = 0;

	virtual std::string do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const;

	virtual std::string do_end_triplets() const = 0;

	virtual std::string do_end_block() const = 0;

	virtual std::string do_end_input() const = 0;

	virtual std::string do_name() const = 0;

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

protected:

	/**
	 * \brief Default method to create a header representation.
	 *
	 * Default implementation for do_header().
	 *
	 * \param[in] track_count Total number of tracks
	 * \param[in] id1         Disc id1
	 * \param[in] id2         Disc id2
	 * \param[in] cddb_id     CDDB id
	 *
	 * \return String representation of the header
	 */
	std::string default_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const;

	/**
	 * \brief Default method to create a triplet representation.
	 *
	 * Default implementation for do_triplet().
	 *
	 * \param[in] arcs          ARCS value
	 * \param[in] confidence    Confidence
	 * \param[in] frame450_arcs ARCS value for frame 450
	 *
	 * \return String representation of a triplet
	 */
	std::string default_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const;

	/**
	 * \brief Create ostringstream for output.
	 *
	 * \return Stringstream for output
	 */
	std::ostringstream create_stream() const;

	/**
	 * \brief Non-const-access to the print layout for subclasses.
	 *
	 * \return The print layout used
	 */
	ARIdLayout* arid_layout_ptr() const;

	/**
	 * \brief Re-usable empty string.
	 *
	 * \return Reference to an empty string
	 */
	const std::string& empty_string() const;

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
	DBAROutputFormat(std::unique_ptr<ARIdLayout> arid_layout);

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
	 *
	 * \return String representation of the header
	 */
	std::string header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const;

	/**
	 * \brief Starting a sequence of triplets.
	 *
	 * \return String to print on start_triplets
	 */
	std::string start_triplets() const;

	/**
	 * \brief Print triplet.
	 *
	 * \param[in] arcs          ARCS value
	 * \param[in] confidence    Confidence
	 * \param[in] frame450_arcs ARCS value for frame 450
	 *
	 * \return String representation of the triplet
	 */
	std::string triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const;

	/**
	 * \brief Ending a sequence of triplets.
	 *
	 * \return String to print on end_triplets
	 */
	std::string end_triplets() const;

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
	 * \brief Name of the format.
	 *
	 * \return Name of the format.
	 */
	std::string name() const;

	/**
	 * \brief Current block counter value.
	 *
	 * \return Current block counter value
	 */
	unsigned block_counter() const;

	/**
	 * \brief Current track counter value.
	 *
	 * \return Current track counter value
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
};


/**
 * \brief Labels for DBAROutputFormat.
 */
enum class DBAR_LABEL : int
{
	DBAR   = 0,
	ID     = 1,
	TRACKS = 2,
	ARCS   = 3,
	CONF   = 4,
	F450   = 5
};


/**
 * \brief Delimiters for DBAROutputFormat.
 */
enum class DBAR_DELIM : int
{
	UNPARSED     =  0,
	DOC_START    =  1,
	DOC_END      =  2,
	DBAR_START   =  3,
	DBAR_END     =  4,
	BLOCK_START  =  5,
	BLOCK_END    =  6,
	BLOCK_DELIM  =  7,
	HEADER_START =  8,
	HEADER_END   =  9,
	TRACKS_START = 10,
	TRACKS_END   = 11,
	TRACK_START  = 12,
	TRACK_END    = 13,
	PROP_DELIM   = 14,
	TRACK_DELIM  = 15,
	LABEL_DELIM  = 16,
	NAME_DELIM   = 17,
	VAL_DELIM    = 18,
};


/**
 * \brief For TextDecoratedFormat.
 */
enum class DBAR_TEXT : int
{
	UNPARSED,
	DELIM1,
	DELIM2,
	DELIM3,
	DELIM4,
	DELIM5,
	BLOCK,
	TRIPLET
};


/**
 * \brief Implements formats 'text_decorated', 'text' and 'raw'.
 */
class TextDecoratedFormat final : public LabelStore<DBAR_TEXT>
								, public PropertyStore<DBAR_TEXT>
								, public DBAROutputFormat
{
	std::string do_start_input() const final;

	std::string do_start_block() const final;

	std::string do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const final;

	std::string do_start_triplets() const final;

	std::string do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const final;

	std::string do_end_triplets() const final;

	std::string do_end_block() const final;

	std::string do_end_input() const final;

	std::string do_name() const final;

public:

	/**
	 * \brief Constructor.
	 */
	TextDecoratedFormat();

	/**
	 * \brief Constructor with labels, properties and layouts.
	 *
	 * \param[in] labels         Labels for text output
	 * \param[in] properties     Properties for text output
	 * \param[in] arid_layout    ARId Layout
	 */
	TextDecoratedFormat(const LabelStore::store_t labels,
			const flags_t properties,
			std::unique_ptr<ARIdLayout> arid_layout);
};


/**
 * \brief Abstract base class for formats 'yaml' and 'json'.
 */
class LabelledDBAROutputFormat  : public DBAROutputFormat
								, public PropertyStore<DBAR_DELIM>
{
	/**
	 * \brief Internal label store.
	 */
	LabelStore<DBAR_LABEL> labels_;

	/**
	 * \brief Internal delim store.
	 */
	LabelStore<DBAR_DELIM> delims_;

	/**
	 * \brief Current indent level.
	 */
	mutable std::string::size_type indent_;

	/**
	 * \brief Amount of incrementing or decrementing the current indent.
	 */
	unsigned indent_step_;


	std::string do_start_input() const override;

	std::string do_start_block() const override;

	std::string do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const override;

	std::string do_start_triplets() const override;

	std::string do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const override;

	std::string do_end_triplets() const override;

	std::string do_end_block() const override;

	std::string do_end_input() const override;

	// do_name() = 0

protected:

	LabelledDBAROutputFormat(const LabelStore<DBAR_LABEL>::store_t& labels,
			const LabelStore<DBAR_DELIM>::store_t& delims,
			const flags_t properties,
			std::unique_ptr<ARIdLayout> arid_layout);

	LabelledDBAROutputFormat(const LabelStore<DBAR_DELIM>::store_t& delims,
			const flags_t properties);

	std::string label(const DBAR_LABEL& label) const;

	std::string value(const std::string& s) const;

	std::string delim(const DBAR_DELIM& delim) const;

	std::string indent() const;

	int inc_indent() const;

	int dec_indent() const;
};


/**
 * \brief Implements format 'yaml'.
 */
class YamlFormat final : public LabelledDBAROutputFormat
{
	std::string do_name() const final;

public:

	/**
	 * \brief Default constructor.
	 */
	YamlFormat();
};


/**
 * \brief Implements format 'json'.
 */
class JsonFormat final : public LabelledDBAROutputFormat
{
	std::string do_name() const final;

public:

	/**
	 * \brief Default constructor.
	 */
	JsonFormat();
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

	void do_start_triplets() final;

	void do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) final;

	void do_end_triplets() final;

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

