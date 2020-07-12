#ifndef __ARCSTOOLS_PRINTERS_HPP__
#define __ARCSTOOLS_PRINTERS_HPP__

/**
 * \file
 *
 * \brief Output formats for printing.
 *
 * Provides different output formats for ARId, ARTriplet and formatted lists
 * of TrackARCSs. The convenience formats for ARProfile and ARBlock are composed
 * from these formats. There is also a format for offsets.
 */

#include <array>                  // for array
#include <cstdint>                // for int32_t
#include <iosfwd>                 // for ostream, size_t
#include <string>                 // for string
#include <vector>                 // for vector

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

namespace arcsapp
{

using arcstk::ARId;
using arcstk::ARTriplet;
using arcstk::ARBlock;
using arcstk::ARResponse;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::Match;
using arcstk::TOC;


// forward declare Print<>
template <typename ...Args>
class Print;

/**
 * \brief Overload << to make each concrete Print<>er be usable with an ostream.
 */
template <typename ...Args>
std::ostream& operator << (std::ostream &stream, Print<Args...> &p)
{
	p.out(stream);
	return stream;
}

/**
 * \brief Print an object that has an overload for operator << and ostream.
 */
template <typename ...Args>
class Print
{
public:

	friend std::ostream& operator << <> (std::ostream &s, Print<Args...> &p);

	/**
	 * \brief Constructor
	 */
	Print(std::tuple<const Args*...> args)
		: pointers_ { args }
	{
		// empty
	}

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~Print() = default;

	/**
	 * \brief Add the objects to print
	 */
	void use(std::tuple<const Args*...> args)
	{
		pointers_ = args;
	}

	/**
	 * \brief Called by overloaded operator <<.
	 *
	 * \param[in] outstream Stream to print to
	 */
	void out(std::ostream &outstream)
	{
		std::ios_base::fmtflags prev_settings = outstream.flags();

		this->do_out(outstream, this->pointers_);

		outstream.flags(prev_settings);
	}

protected:

	/**
	 * \brief Do assertions for input arguments.
	 *
	 * \param[in] t Tuple of input arguments
	 */
	virtual void assertions(const std::tuple<const Args*...> &t) const
	= 0;

private:

	/**
	 * \brief Store pointers to printable objects
	 */
	std::tuple<const Args*...> pointers_;

	/**
	 * \brief Implements out()
	 *
	 * \param[in] o Stream to print to
	 * \param[in] t Tuple of data pointers to print
	 */
	virtual void do_out(std::ostream &o, const std::tuple<const Args*...> &t)
	= 0;
};


/**
 * \brief Abstract base class for output formats of ARTriplet.
 */
class ARTripletFormat final : virtual protected WithInternalFlags
							, public Print<int, ARTriplet>
{
public:

	/**
	 * \brief Constructor.
	 */
	ARTripletFormat();

private:

	void assertions(const std::tuple<const int*, const ARTriplet*> &t) const
		override;
	// from Print

	void do_out(std::ostream &out,
			const std::tuple<const int*, const ARTriplet*> &t) override;
	// from Print
};


/**
 * \brief Aliases for classes printing an ARId.
 */
using ARIdPrinter = Print<ARId, std::string>;


/**
 * \brief Prints ARId in specified layout.
 */
class ARIdFormat final  : public WithARId
						, public ARIdPrinter
{
public:

	/**
	 * \brief Constructor setting all flags with empty ARId and no prefix.
	 *
	 * \param[in] id          Set to TRUE for printing the ID
	 * \param[in] url         Set to TRUE for printing the URL
	 * \param[in] filename    Set to TRUE for printing the filename
	 * \param[in] track_count Set to TRUE for printing the track_count
	 * \param[in] disc_id_1   Set to TRUE for printing the disc id1
	 * \param[in] disc_id_2   Set to TRUE for printing the disc id2
	 * \param[in] cddb_id     Set to TRUE for printing the cddb id
	 */
	ARIdFormat(const bool id, const bool url, const bool filename,
			const bool track_count, const bool disc_id_1, const bool disc_id_2,
			const bool cddb_id);

	/**
	 * \brief Default constructor.
	 *
	 * Sets empty ARId, no prefix and all formatting flags to FALSE except URL.
	 */
	ARIdFormat() : ARIdFormat(true,
			false, false, false, false, false, false)
	{ /* empty */ };

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARIdFormat() noexcept;

private:

	void assertions(const std::tuple<const ARId*, const std::string*> &t) const
		override;
	// from Print

	void do_out(std::ostream &o, const std::tuple<const ARId*,
			const std::string*> &t)
		override; // from Print
};


/**
 * \brief Alias for classes printing checksum calculation results.
 */
using ChecksumsResultPrinterBase =
	Print<Checksums, std::vector<std::string>, TOC, ARId, bool>;


/**
 * \brief Inherit assertions common to all Checksum printers.
 */
class ChecksumsResultPrinter : public ChecksumsResultPrinterBase
{
public:

	/**
	 * \brief Default constructor
	 */
	ChecksumsResultPrinter();

	using ChecksumsResultPrinterBase::ChecksumsResultPrinterBase;

protected:

	void assertions(
		const std::tuple<const Checksums*, const std::vector<std::string>*,
			const TOC*, const ARId*, const bool*> &t) const override;
	// from Print
};


/**
 * \brief Simple table format for album-based Checksums.
 *
 * Checksums are columns and tracks are rows.
 */
class AlbumChecksumsTableFormat final   : public TypedColsTableBase
										, public ChecksumsResultPrinter
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] label    Set to TRUE for printing column titles
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 * \param[in] coldelim Set column delimiter
	 */
	AlbumChecksumsTableFormat(const bool label, const bool track,
			const bool offset, const bool length, const bool filename,
			const std::string &coldelim);

	/**
	 * \brief Virtual default destructor.
	 */
	~AlbumChecksumsTableFormat() noexcept override;

private:

	void do_init(const int rows, const int cols) override;
	// from StringTableStructure

	int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;
	// from TypedColsTableBase

	void do_out(std::ostream &o,
		const std::tuple<const Checksums*, const std::vector<std::string>*,
		const TOC*, const ARId*, const bool*> &t) override;
	// from Print
};


/**
 * \brief Simple table format for album-based Checksums.
 *
 * Checksums are rows and tracks are cols.
 */
class AlbumTracksTableFormat final  : public TypedRowsTableBase
									, public ChecksumsResultPrinter
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] label    Set to TRUE for printing row labels
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 * \param[in] coldelim Set column delimiter
	 */
	AlbumTracksTableFormat(const bool label, const bool track,
			const bool offset, const bool length, const bool filename,
			const std::string &coldelim);

private:

	void do_init(const int rows, const int cols) override;
	// from StringTableBase

	void do_out(std::ostream &o,
		const std::tuple<const Checksums*, const std::vector<std::string>*,
			const TOC*, const ARId*, const bool*> &t) override;
	// from Print
};


/**
 * \brief Alias for classes printing match results.
 */
using MatchResultPrinterBase =
	Print<Checksums, std::vector<std::string>, std::vector<Checksum>, Match,
	int, bool, TOC, ARId>;


/**
 * \brief Inherit assertions common to all Checksum printers.
 */
class MatchResultPrinter : public MatchResultPrinterBase
{
public:

	/**
	 * \brief Default constructor
	 */
	MatchResultPrinter();

	using MatchResultPrinterBase::MatchResultPrinterBase;

	/**
	 * \brief Set the symbol to be printed on identity of two checksum values.
	 *
	 * When some values do not match, the respective values are printed.
	 *
	 * \param[in] match_symbol The symbol to represent a match
	 */
	void set_match_symbol(const std::string &match_symbol);

	/**
	 * \brief The symbol to be printed on identity of two checksum values.
	 *
	 * When some values do not match, the respective values are printed.
	 *
	 * \return Match symbol
	 */
	const std::string& match_symbol() const;

protected:

	void assertions(
		const std::tuple<const Checksums*, const std::vector<std::string>*,
			const std::vector<Checksum>*, const Match*, const int*, const bool*,
			const TOC*, const ARId*> &t) const
		override;
	// from Print

private:

	/**
	 * \brief The symbol to be printed on identity of two checksum values.
	 */
	std::string match_symbol_;
};


/**
 * \brief Simple table format for album-based matches.
 */
class AlbumMatchTableFormat final   : public TypedColsTableBase
									, public MatchResultPrinter
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] label    Set to TRUE for printing column titles
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 * \param[in] coldelim Set column delimiter
	 */
	AlbumMatchTableFormat(const bool track, const bool label,
			const bool offset, const bool length, const bool filename,
			const std::string &coldelim);

	/**
	 * \brief Virtual default destructor.
	 */
	~AlbumMatchTableFormat() noexcept override;

private:

	void do_init(const int rows, const int cols) override;
	// from StringTableStructure

	int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;
	// from TypedColsTableBase

	void do_out(std::ostream &out,
			const std::tuple<const Checksums*, const std::vector<std::string>*,
			const std::vector<Checksum>*, const Match*, const int*, const bool*,
			const TOC*, const ARId*> &t)
		override;
	// from Print
};

} // namespace arcsapp

#endif

