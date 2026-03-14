#ifndef ARCSTOOLS_TOOLS_ARID_HPP_
#define ARCSTOOLS_TOOLS_ARID_HPP_
/**
 * \file
 *
 * \brief Formatter for ARId objects.
 */

#include <cstdint>      // for uint32_t
#include <array>        // for array
#include <memory>       // for unique_ptr
#include <ostream>      // for ostream
#include <string>       // for string

#ifndef LIBARCSTK_IDENTIFIER_HPP_
#include <arcstk/identifier.hpp>  // for ARId
#endif

#ifndef ARCSTOOLS_LAYOUTS_HPP_
#include "layouts.hpp"      // for Layout
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief Tools for managing ARIds.
 */
namespace arid
{

using arcstk::ARId;
using arcstk::ToC;


/**
 * \brief Show flags of ARIdLayout
 */
enum class ARID_FLAG : int
{
	ID       = 0,
	URL      = 1,
	FILENAME = 2,
	TRACKS   = 3,
	ID1      = 4,
	ID2      = 5,
	CDDBID   = 6
};


/**
 * \brief Interface for formatting ARIds.
 */
using IdLayout = Layout<std::string, arcstk::ARId, std::string>;


/**
 * \brief Interface for formatting ARId instances for output.
 */
class ARIdLayout : public PropertyStore<ARID_FLAG>
				 , public IdLayout // TODO Do also Settings!
				 , public LabelStore<ARID_FLAG>
{
public:

	/**
	 * \brief Default constructor.
	 *
	 * Sets all formatting flags to TRUE
	 */
	ARIdLayout();

	/**
	 * \brief Constructor setting all flags.
	 *
	 * \param[in] labels      Set to TRUE for printing the field labels
	 * \param[in] id          Set to TRUE for printing the ID
	 * \param[in] url         Set to TRUE for printing the URL
	 * \param[in] filename    Set to TRUE for printing the filename
	 * \param[in] track_count Set to TRUE for printing the track_count
	 * \param[in] disc_id_1   Set to TRUE for printing the disc id1
	 * \param[in] disc_id_2   Set to TRUE for printing the disc id2
	 * \param[in] cddb_id     Set to TRUE for printing the cddb id
	 */
	ARIdLayout(const bool labels, const bool id, const bool url,
			const bool filename, const bool track_count, const bool disc_id_1,
			const bool disc_id_2, const bool cddb_id);

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~ARIdLayout() noexcept;

	/**
	 * \brief Returns TRUE iff instance is configured to print field labels.
	 *
	 * \return TRUE iff instance is configured to print field labels
	 */
	bool fieldlabels() const;

	/**
	 * \brief Set to TRUE to print field labels.
	 *
	 * \param[in] labels Flag to indicate that field labels have to be printed
	 */
	void set_fieldlabels(const bool labels);

	/**
	 * \brief Label for the specified flag.
	 *
	 * \param[in] flag Flag to get label for
	 *
	 * \return Label for \c flag
	 */
	auto label(const ARID_FLAG flag) const -> std::string;

	/**
	 * \brief Set label for the specified flag.
	 *
	 * \param[in] flag  Flag to set label for
	 * \param[in] label Label to set
	 */
	void set_label(const ARID_FLAG flag, const std::string& label);

	/**
	 * \brief Deep copy of this instance.
	 *
	 * \return Deep copy of this instance
	 */
	std::unique_ptr<ARIdLayout> clone() const;
	// TODO Implement clone_base()

private:

	/**
	 * \brief Iterable aggregate of the defined flags.
	 *
	 * Order matches definition order in ARID_FLAG.
	 */
	const std::array<ARID_FLAG, 7> show_flags_
	{
		ARID_FLAG::ID,
		ARID_FLAG::URL,
		ARID_FLAG::FILENAME,
		ARID_FLAG::TRACKS,
		ARID_FLAG::ID1,
		ARID_FLAG::ID2,
		ARID_FLAG::CDDBID
	};

	/**
	 * \brief Row label for the defined flags.
	 *
	 * Order matches definition order in ARID_FLAG.
	 */
	std::array<std::string, 7> labels_ // TODO LabelStore
	{
		"ID",
		"URL",
		"Filename",
		"Tracks",
		"ID1",
		"ID2",
		"CDDB ID"
	};

	/**
	 * \brief Field labels.
	 */
	bool field_labels_;

	virtual std::unique_ptr<ARIdLayout> do_clone() const
	= 0;

protected:

	/**
	 * \brief Worker: print the the sub-ids as part of an ARId.
	 *
	 * \param[in] id Id to print as part of an ARId
	 *
	 * \return Hexadecimal ARId-conforming representation of a 32bit unsigned.
	 */
	std::string hex_id(const uint32_t id) const;

	/**
	 * \brief Getter for the show flags.
	 *
	 * \return Show flags
	 */
	auto show_flags() const -> decltype( show_flags_ );

	/**
	 * \brief Getter for the flag labels.
	 *
	 * \return Flag labels
	 */
	auto labels() const -> decltype( labels_ );

	/**
	 * \brief Turn flag to an array index.
	 *
	 * \param[in] flag Flag to turn to an array index
	 *
	 * \return Array index this flag points to
	 */
	auto array_idx(const ARID_FLAG flag) const -> unsigned;
};


/**
 * \brief Table-style layout for ARId information.
 */
class ARIdTableLayout final : public ARIdLayout
{
public:

	using ARIdLayout::ARIdLayout;

private:

	std::unique_ptr<ARIdLayout> do_clone() const override;

	std::string do_format(InputTuple t) const override;
};


/**
 * \brief An ARId accompanied by a layout and an optional URL prefix.
 *
 * This object contains all information necessary to be printed.
 */
class RichARId final
{
	arcstk::ARId id_;

	std::unique_ptr<ARIdLayout> layout_;

	std::string alt_prefix_;

public:

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id      ARId to print
	 * \param[in] layout  Layout to use for printing
	 */
	RichARId(const arcstk::ARId& id, std::unique_ptr<ARIdLayout> layout);

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id         ARId to print
	 * \param[in] layout     Layout to use for printing
	 * \param[in] alt_prefix Optional alternative URL prefix
	 */
	RichARId(const arcstk::ARId& id, std::unique_ptr<ARIdLayout> layout,
			const std::string& alt_prefix);

	/**
	 * \brief ARId of this instance.
	 *
	 * \return ARId of this instance
	 */
	const arcstk::ARId& id() const;

	/**
	 * \brief ARIdLayout of this instance.
	 *
	 * \return Layout used to print ARId
	 */
	const ARIdLayout& layout() const;

	/**
	 * \brief Alternative URL prefix used when printing the ARId.
	 *
	 * \return Alterntaive URL prefix
	 */
	const std::string& alt_prefix() const;
};


/**
 * \brief Stream insertion operator for RichARId.
 *
 * \param[in] o Stream to output object to
 * \param[in] a RichARId to print to stream
 *
 * \return Stream with \p a inserted
 */
std::ostream& operator << (std::ostream& o, const RichARId& a);


/**
 * \brief Build an ARId enriched with print information.
 *
 * \param[in] toc        ToC by which \p arid was created
 * \param[in] arid       Actual ARId
 * \param[in] alt_prefix Alternative URL prefix
 * \param[in] layout     Layout to print ARId
 *
 * \return ARId container
 */
RichARId build_id(const ToC* toc, const ARId& arid,
		const std::string& alt_prefix, const ARIdLayout& layout);


/**
 * \brief Validate the input objects common to every result.
 *
 * Throws if validation fails.
 *
 * \param[in] arid         ARId to validate against the other data
 * \param[in] total_tracks Number of tracks as resulted
 * \param[in] toc          ToC as resulted
 *
 * \throws invalid_argument If validation fails
 */
void validate(const ARId& arid, const std::size_t total_tracks, const ToC* toc);


} // namespace arid
} // namespace v_1_0_0
} // namespace arcsapp

#endif

