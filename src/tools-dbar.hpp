#ifndef ARCSTOOLS_TOOLS_DBAR_HPP_
#define ARCSTOOLS_TOOLS_DBAR_HPP_
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

#ifndef LIBARCSTK_DBAR_HPP_
#include <arcstk/dbar.hpp>   // for DBAR, ParseHandler, ParseErrorHandler
#endif

#ifndef ARCSTOOLS_LAYOUTS_HPP_
#include "layouts.hpp"       // for Layout
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{

// forward declarations

namespace arid
{
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
	// arcstk::ParseHandler

	virtual std::string do_start_input() const = 0;

	virtual std::string do_start_block() const = 0;

	virtual std::string do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const = 0;

	virtual std::string do_start_triplets() const = 0;

	virtual std::string do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const = 0;

	virtual std::string do_end_triplets() const = 0;

	virtual std::string do_end_block() const = 0;

	virtual std::string do_end_input() const = 0;

	//

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
	std::string default_id(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const;

	/**
	 * \brief Default method to create an ARCS.
	 *
	 * \param[in] number ARCS value from parser
	 *
	 * \return String representation of the parsed ARCS
	 */
	std::string default_arcs(const uint32_t number) const;

	/**
	 * \brief Default method to create a confidence value.
	 *
	 * \param[in] number Confidence value from parser
	 *
	 * \return String representation of the parsed value
	 */
	std::string default_confidence(const unsigned number) const;

	/**
	 * \brief Default method to create the ARCS of frame 450.
	 *
	 * \param[in] number ARCS value from parser
	 *
	 * \return String representation of the parsed ARCS
	 */
	std::string default_f450_arcs(const uint32_t number) const;

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
	 * \brief Constructor with layouts.
	 *
	 * \param[in] arid_layout    Layout for ARIds
	 */
	explicit DBAROutputFormat(std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Default constructor.
	 */
	DBAROutputFormat();

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
 * \brief Data entities of a DBAR object.
 */
enum class DBAR_ENTITY : uint8_t
{
	DBAR   = 0,
	ID     = 1,
	TRACKS = 2,
	TRACK  = 3,
	ARCS   = 4,
	CONF   = 5,
	F450   = 6
};


/**
 * \brief Delimiters for syntactic elements of a DBAR.
 */
enum class DBAR_DELIM : int
{
	UNPARSED     =  0,  /* unparsed value */
	DOC_START    =  1,  /* on document start */
	DOC_END      =  2,  /* on document end */
	DBAR_START   =  3,  /* on DBAR object start */
	DBAR_END     =  4,  /* on DBAR object end */
	BLOCK_START  =  5,  /* start of a block */
	BLOCK_END    =  6,  /* end of a block */
	BLOCK_DELIM  =  7,  /* between two blocks */
	HEADER_START =  8,  /* start of a header */
	HEADER_END   =  9,  /* end of a header */
	TRACKS_START = 10,  /* start of a list of tracks/triplets */
	TRACKS_END   = 11,  /* end of a list of tracks/triplets */
	TRACK_START  = 12,  /* start of a triplet */
	TRACK_END    = 13,  /* end of a triplet */
	PROP_DELIM1  = 14,  /* between first and second element of a triplet */
	PROP_DELIM2  = 15,  /* between second and third element of a triplet */
	TRACK_DELIM  = 16,  /* between triplets */
	LABEL_DELIM  = 17,  /* between a label and a value */
	NAME_DELIM   = 18,  /* start/end delimiters for a name */
	VAL_DELIM    = 19   /* start/end delimiters for a value */
};


/**
 * \brief Abstract base class for formatting DBAR objects.
 */
class DBARBaseFormat : public DBAROutputFormat
					 , public LabelStore<DBAR_DELIM>
					 , public PropertyStore<DBAR_DELIM>
{
	/**
	 * \brief Current indent level.
	 */
	mutable std::string::size_type indent_;

	/**
	 * \brief Amount of incrementing or decrementing the current indent.
	 */
	unsigned indent_step_;

	// arcstk::ParseHandler

	std::string do_start_input() const override;

	std::string do_end_input() const override;

	std::string do_start_block() const override;

	std::string do_end_block() const override;

	std::string do_start_triplets() const override;

	std::string do_end_triplets() const override;

	std::string do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const override;

	std::string do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const override;

	// do_name

	virtual std::string do_id(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const;

	virtual std::string do_arcs(const uint32_t number) const;

	virtual std::string do_confidence(const unsigned number) const;

	virtual std::string do_f450_arcs(const uint32_t number) const;

	virtual std::string do_delim(const DBAR_DELIM delim) const;

protected:

	/**
	 * \brief Current indent.
	 *
	 * \return Indent string with current indent width
	 */
	std::string indent() const;

	/**
	 * \brief Increase indent by 1 step.
	 *
	 * \return New current indent
	 */
	int inc_indent() const;

	/**
	 * \brief Decrease indent by 1 step.
	 *
	 * \return New current indent
	 */
	int dec_indent() const;

public:

	/**
	 * \brief Constructor with labels, properties and layout.
	 *
	 * \param[in] labels       Labels for text output
	 * \param[in] properties   Properties for text output
	 * \param[in] arid_layout  ARId Layout
	 * \param[in] indent_start Initial indent
	 * \param[in] indent_step  Size of a single indentation step
	 */
	DBARBaseFormat(const LabelStore::store_t& labels, const flags_t properties,
			std::unique_ptr<ARIdLayout> arid_layout,
			const std::string::size_type indent_start,
			const unsigned indent_step);

	/**
	 * \brief Constructor with labels and layout.
	 *
	 * All properties represented by a key in \c labels are set to TRUE.
	 *
	 * \param[in] labels       Labels for text output
	 * \param[in] arid_layout  ARId Layout
	 * \param[in] indent_start Initial indent
	 * \param[in] indent_step  Size of a single indentation step
	 */
	DBARBaseFormat(const LabelStore::store_t& labels,
			std::unique_ptr<ARIdLayout> arid_layout,
			const std::string::size_type indent_start,
			const unsigned indent_step);

	/**
	 * \brief Constructor with labels.
	 *
	 * All properties represented by a key in \c labels are set to TRUE, no
	 * ARIDLayout is used.
	 *
	 * \param[in] labels       Labels for text output
	 * \param[in] indent_start Initial indent
	 * \param[in] indent_step  Size of a single indentation step
	 */
	explicit DBARBaseFormat(const LabelStore::store_t& labels,
			const std::string::size_type indent_start,
			const unsigned indent_step);

	/**
	 * \brief Constructor.
	 *
	 * Enable parameterless constructors in subclasses.
	 */
	DBARBaseFormat();

	/**
	 * \brief Get delimiter.
	 *
	 * Note: this is just an alias for the inherited function
	 * LabelStore<DBAR_DELIM>::label(const DBAR_DELIM).
	 *
	 * \param[in] delim Delimiter symbol
	 *
	 * \return Delimiter string for symbol
	 */
	std::string delim(const DBAR_DELIM delim) const;
};


/**
 * \brief Implements formats 'text_decorated', 'text' and 'raw'.
 */
class TextDecoratedFormat final : public DBARBaseFormat
{
	// DBAROutputFormat

	std::string do_name() const final;

public:

	/**
	 * \brief Constructor with delimiters, properties and layout.
	 *
	 * \param[in] delims      Delimiters for text output
	 * \param[in] properties  Properties for text output
	 * \param[in] arid_layout ARId Layout
	 */
	TextDecoratedFormat(const LabelStore::store_t& delims,
			const flags_t properties,
			std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Constructor with delimiters and layout.
	 *
	 * All properties represented by a key in \c delims are set to TRUE.
	 *
	 * \param[in] delims      Delimiters for text output
	 * \param[in] arid_layout ARId Layout
	 */
	TextDecoratedFormat(const LabelStore::store_t& delims,
		std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Constructor with delimiters.
	 *
	 * All properties represented by a key in \c delims are set to TRUE, no
	 * ARIDLayout is used.
	 *
	 * \param[in] delims Delimiters for text output
	 */
	explicit TextDecoratedFormat(const LabelStore::store_t& delims);

	/**
	 * \brief Constructor.
	 */
	TextDecoratedFormat();
};


/**
 * \brief Abstract base class for formats 'yaml' and 'json'.
 */
class LabelledDBAROutputFormat : public DBARBaseFormat
{
	/**
	 * \brief Internal label store.
	 */
	LabelStore<DBAR_ENTITY> labels_;

	// DBAROutputFormat

	// do_name() = 0

	// DBARBaseFormat

	std::string do_id(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const override;

	std::string do_arcs(const uint32_t number) const override;

	std::string do_confidence(const unsigned number) const override;

	std::string do_f450_arcs(const uint32_t number) const override;

	std::string do_delim(const DBAR_DELIM delim) const override;

	//

	virtual std::string do_label(const DBAR_ENTITY& label) const;

	virtual std::string do_value(const std::string& s) const;

protected:

	/**
	 * \brief Constructor with labels, delimiters, properties and layout.
	 *
	 * \param[in] labels      Labels for text output
	 * \param[in] delims      Delimiters for text output
	 * \param[in] properties  Properties for text output
	 * \param[in] arid_layout ARId Layout
	 */
	LabelledDBAROutputFormat(const LabelStore<DBAR_ENTITY>::store_t& labels,
			const LabelStore<DBAR_DELIM>::store_t& delims,
			const flags_t properties,
			std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Constructor with delimiters.
	 *
	 * Uses no ARIdLayout and default labels.
	 *
	 * \param[in] delims Delimiters for text output
	 */
	explicit LabelledDBAROutputFormat(
			const LabelStore<DBAR_DELIM>::store_t& delims);

	/**
	 * \brief Get label for entity.
	 *
	 * \param[in] entity Entitiy to get label for
	 *
	 * \return Printable label for \c entity
	 */
	std::string label(const DBAR_ENTITY& entity) const;

	/**
	 * \brief Format \c s for a value.
	 *
	 * \param[in] s String to format as value
	 *
	 * \return Printable value representing s
	 */
	std::string value(const std::string& s) const;
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
	~PrintParseHandler() noexcept final;

	/**
	 * \brief Sets the format for printing DBAR objects.
	 *
	 * \param[in] format The print format to use for DBAR objects
	 */
	void set_format(std::unique_ptr<DBAROutputFormat> format);

	/**
	 * \brief Read-access to the print format used for DBAR objects.
	 *
	 * \return The print format used for DBAR objects
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

