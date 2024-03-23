#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#define __ARCSTOOLS_TOOLS_PARSE_HPP__

/**
 * \file
 *
 * \brief Content and error handlers for ARParsers.
 *
 * Provides parser handlers for ARParsers.
 */

#include <cstddef>           // for size_t
#include <cstdint>           // for uint32_t, uint8_t
#include <exception>         // for exception
#include <istream>           // for istream
#include <memory>            // for unique_ptr
#include <streambuf>         // for basic_streambuf
#include <stdexcept>         // for runtime_error, domain_error
#include <string>            // for string, char_traits
#include <utility>           // for move, swap
#include <vector>            // for vector

#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"            // for Layout
#endif

namespace arcsapp
{

class Application;
class ARIdLayout;
class DBARTripletLayout;

using arcstk::DBAR;
using arcstk::ParseHandler;
using arcstk::ParseErrorHandler;


/**
 * \brief Wrap a vector in an istream.
 */
template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class VectorIStream : public std::basic_streambuf<CharT, TraitsT>
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] v The vector to wrap
	 */
	explicit VectorIStream(std::vector<CharT> &v)
	{
		this->setg(v.data(), v.data(), v.data() + v.size());
	}
};


/**
 * \brief Buffered binary read access to stdin.
 */
class StdIn final
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] buf_size Buffer size in bytes
	 */
	explicit StdIn(const std::size_t buf_size);

	/**
	 * \brief Reads stdin bytes in binary mode to a vector<char>.
	 *
	 * \return Bytes from stdin
	 */
	std::vector<char> bytes();

	/**
	 * \brief Size of read buffer in bytes.
	 *
	 * \return Buffer size in bytes
	 */
	std::size_t buf_size() const;

private:

	/**
	 * \brief Maximal number of kilobytes to accept as input.
	 */
	static const int MAX_KB_ = 50;

	/**
	 * \brief Bytes per read
	 */
	const std::size_t buf_size_;
};


unsigned read_from_stdin(const std::size_t amount_of_bytes, ParseHandler* p,
		ParseErrorHandler* e);


/**
 * \brief Content handler that just prints the parsed content immediately.
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
class DBARTripletLayout : protected WithInternalFlags
						, public TripletLayout
{
public:

	using TripletLayout::Layout;

private:

	// no assertions()

	std::string do_format(InputTuple t) const override;
};

} // namespace arcsapp

#endif

