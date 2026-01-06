#ifndef __ARCSTOOLS_TOOLS_INPUT_HPP__
#define __ARCSTOOLS_TOOLS_INPUT_HPP__

/**
 * \file
 *
 * \brief Tools for input management on cli and files.
 */

#include <any>                    // for any
#include <functional>             // for function
#include <streambuf>              // for basic_streambuf
#include <string>                 // for string, char_traits

#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>        // for DBAR, ParseHandler, ParseErrorHandler
#endif
#ifndef __LIBARCSTK_VERIFY_HPP__
#include <arcstk/verify.hpp>      // for ChecksumSource, ChecksumSourceOf
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>     // for ARCS_LOG,...
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace input
{

using arcstk::ARId;
using arcstk::Checksum;
using arcstk::ChecksumSource;
using arcstk::ChecksumSourceOf;
using arcstk::DBAR;


// for parsing DBARs


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
	explicit VectorIStream(std::vector<CharT>& v)
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


/**
 * \internal
 *
 * \brief Worker: Read an amount of bytes from stdin to a DBAR parser.
 *
 * \param[in] amount_of_bytes Total number of bytes to read
 * \param[in] p               ParseHandler to parse the bytes
 * \param[in] e               ParseErrorHandler to handle errors
 *
 * \return Total number of bytes parsed
 */
unsigned read_from_stdin(const std::size_t amount_of_bytes,
		arcstk::ParseHandler* p,
		arcstk::ParseErrorHandler* e);


// for parsing anything


/**
 * \brief Parse \p list as a sequence of strings separated by \p delim and call
 * \p entry_hook on each of them.
 *
 * \param[in] list       Input string to parse as a list
 * \param[in] delim      Delimiter for list entries
 * \param[in] entry_hook Call this function on each entry
 */
void parse_list(const std::string& list, const char delim,
		std::function<void(const std::string& s)> entry_hook);


/**
 * \brief Parse \p list as a sequence of strings separated by \p delim and
 * convert each entry by \p convert_func.
 *
 * \tparam T Type of requested objects
 *
 * \param[in] list         Input string to parse as a list
 * \param[in] delim        Delimiter for list entries
 * \param[in] convert_func Function to convert std::string to T
 *
 * \return Sequence of input values converted from strings
 */
template <typename T>
inline std::vector<T> parse_list_to_objects(const std::string& list,
		const char delim,
		const std::function<T(const std::string& s)>& convert_func)
{
	auto results = std::vector<T> {};
	// TODO reserve default?

	parse_list(list, delim,
			[&convert_func,&results](const std::string& s)
			{
				results.emplace_back(convert_func(s));
			});

	return results;
}


/**
 * \brief Abstract base class for string parsers.
 *
 * The result class is a std::any that can be put into a Configuration.
 */
class StringParser
{
	virtual std::string start_message() const
	= 0;

	virtual std::any do_parse(const std::string& s) const
	= 0;

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~StringParser() = default;

	/**
	 * \brief Parse input string to object.
	 *
	 * \param[in] s Input string to be parsed
	 *
	 * \return Result object
	 */
	std::any parse(const std::string& s) const;
};


/**
 * \brief Abstract base class for for option value string parsers.
 *
 * \tparam T Result type
 */
template <typename T>
class InputStringParser : public StringParser
{
	/**
	 * \brief Parsing result for empty input.
	 *
	 * Default implementation returns an instance created by default
	 * constructor.
	 *
	 * \return Result instance for an empty input string
	 */
	virtual auto do_parse_empty() const -> T
	{
		ARCS_LOG(DEBUG1) << "Empty parser input, return default object";
		return T { /* empty */ }; // TODO Use declval?
	}

	/**
	 * \brief Parsing result for non-empty input.
	 *
	 * \return Result instance for a non-empty input string
	 *
	 * \throws std::runtime_error If parsing fails
	 */
	virtual auto do_parse_nonempty(const std::string& s) const -> T
	= 0;

	// StringParser

	std::any do_parse(const std::string& s) const final
	{
		if (s.empty())
		{
			return this->do_parse_empty();
		}

		return this->do_parse_nonempty(s);
	}
};


/**
 * \brief Parser for a dBAR response, either from a file or from stdin.
 *
 * Accepts binary input for option VERIFY::RESPONSEFILE.
 */
class DBARParser final : public InputStringParser<DBAR>
{
	/**
	 * \brief Load DBAR from file or from stdin.
	 *
	 * In case the filename is empty, input is expected from stdin.
	 *
	 * \param[in] file The name of the response file
	 */
	DBAR load_data(const std::string& file) const;

	// InputStringParser

	std::string start_message() const final;

	DBAR do_parse_empty() const final;

	DBAR do_parse_nonempty(const std::string& s) const final;
};


/**
 * \brief Type for list of ARCS values.
 */
using ChecksumValuesType = std::vector<uint32_t>;


/**
 * \brief Parser for a list of ARCS values.
 *
 * Accepts a comma-separated list of 32 bit hexadecimal values as input.
 * Does not support blocks, i.e. all input values are considered as part of
 * block 0. The class is therefore only suitable to represent a single sequence
 * of contiguous tracks of a single album.
 */
class ChecksumValuesParser final : public InputStringParser<ChecksumValuesType>
{
	std::string start_message() const final;

	ChecksumValuesType do_parse_nonempty(const std::string& s) const final;
};


/**
 * \brief Compatibility wrapper for a list of ARCS values.
 */
class ChecksumValuesSource final : public ChecksumSourceOf<ChecksumValuesType>
{
	ARId do_id(const ChecksumSource::size_type block_idx) const final;
	Checksum do_checksum(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type idx) const final;
	const uint32_t& do_arcs_value(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_confidence(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_frame450_arcs_value(
			const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	std::size_t do_size(const ChecksumSource::size_type block_idx) const final;
	std::size_t do_size() const final;
	std::unique_ptr<ChecksumSource> do_clone() const final;

public:

	using ChecksumSourceOf::ChecksumSourceOf;
	using ChecksumSourceOf::operator=;
};


/**
 * \brief Dummy source for providing only empty checksums.
 */
class EmptyChecksumSource final : public ChecksumSource
{
	ARId do_id(const ChecksumSource::size_type block_idx) const final;
	Checksum do_checksum(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type idx) const final;
	const uint32_t& do_arcs_value(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_confidence(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_frame450_arcs_value(
			const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	std::size_t do_size(const ChecksumSource::size_type block_idx) const final;
	std::size_t do_size() const final;
	std::unique_ptr<ChecksumSource> do_clone() const final;

public:

	/**
	 * \brief Reference object for a checksum value of 0.
	 */
	static constexpr auto zero = uint32_t { 0 };

	/**
	 * \brief Default constructor.
	 */
	EmptyChecksumSource();
};

} // namespace input
} // namespace v_1_0_0
} // namespace arcsapp

#endif

