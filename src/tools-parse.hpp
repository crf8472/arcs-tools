#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#define __ARCSTOOLS_TOOLS_PARSE_HPP__

/**
 * \file
 *
 * \brief Content and error handlers for ARParsers.
 *
 * Provides parser handlers for ARParsers.
 */

#include <cerrno>            // for errno
#include <cstdint>           // for uint32_t, uint8_t
#include <cstdio>            // for ferror, stdin, feof, freopen, fread, EOF
#include <cstring>           // for strerror
#include <fstream>           // for ofstream, ostream
#include <memory>            // for unique_ptr
#include <sstream>           // for basic_stringstream
#include <stdexcept>         // for runtime_error, domain_error
#include <string>            // for string
#include <utility>           // for move, swap
#include <vector>            // for vector

#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif

namespace arcsapp
{

class ARIdLayout;
class ARTripletLayout;
class Application;

using arcstk::ContentHandler;


/**
 * \brief Wrap a vector in an istream.
 *
 * For parsing stdin.
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
 * \brief Represents limited access to stdin.
 *
 * For parsing stdin.
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
	ARParserContentPrintHandler() : ARParserContentPrintHandler(std::string{})
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
};


/**
 * \brief Parser for dBAR-\*.bin files.
 *
 * \details
 *
 * This class parses dBAR-\*.bin files saved by the actual ripper software
 * or achieved by an HTTP request to AccurateRip. Those files are just the byte
 * stream of the AccurateRip response persisted to the file system.
 *
 * The parser can be reused to parse multiple files subsequently.
 */
class ARFileParser final : public arcstk::ARStreamParser
{
public:

	/**
	 * \brief Constructor.
	 */
	ARFileParser();

	/**
	 * \brief Move Constructor
	 *
	 * \param[in] rhs Instance to move
	 */
	ARFileParser(ARFileParser &&rhs) noexcept;

	/**
	 * \brief Constructor for specific file.
	 *
	 * \param[in] filename Name of the file to parse
	 */
	explicit ARFileParser(const std::string &filename);

	/**
	 * \brief Set the file to be parsed.
	 *
	 * \param[in] filename Name of the file to parse
	 */
	void set_file(const std::string &filename);

	/**
	 * \brief Name of the file to parse.
	 *
	 * \return Name of the file that is parsed when
	 * \link ARStreamParser::parse() parse() \endlink is called.
	 */
	std::string file() const noexcept;


	ARFileParser& operator = (ARFileParser &&rhs) noexcept;

	friend void swap(ARFileParser &lhs, ARFileParser &rhs)
	{
		lhs.swap(rhs);
	}

	// non-copyable class
	ARFileParser(const ARFileParser &rhs) = delete;
	ARFileParser& operator = (const ARFileParser &rhs) = delete;


private:

	uint32_t do_parse() final;

	void do_swap(ARStreamParser &rhs) final;

	void on_catched_exception(std::istream &istream,
			const std::exception &e) const final;

	/**
	 * \brief Internal filename representation
	 */
	std::string filename_;
};


/**
 * \brief Parser for AccurateRip response as a binary stream on stdin.
 */
class ARStdinParser final : public arcstk::ARStreamParser
{
public:

	/**
	 * \brief Default constructor.
	 */
	ARStdinParser();

	/**
	 * \brief Move Constructor
	 *
	 * \param[in] rhs Instance to move
	 */
	ARStdinParser(ARStdinParser &&rhs) noexcept;


	ARStdinParser& operator = (ARStdinParser &&rhs) noexcept;

	friend void swap(ARStdinParser &lhs, ARStdinParser &rhs)
	{
		lhs.swap(rhs);
	}

	// non-copyable class
	ARStdinParser(const ARStdinParser &rhs) = delete;
	ARStdinParser& operator = (const ARStdinParser &rhs) = delete;


private:

	uint32_t do_parse() final;

	void on_catched_exception(std::istream &istream,
			const std::exception &e) const final;
};

} // namespace arcsapp

#endif

