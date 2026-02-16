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

/**
 * \brief Tools for parsing complex option values and arguments.
 */
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
 * \param[in]  list         Input string to parse as a list
 * \param[in]  delim        Delimiter for list entries
 * \param[in]  convert_func Function to convert std::string to T
 * \param[out] count        Total number of list entries parsed
 *
 * \return Sequence of input values converted from strings
 */
template <typename T>
inline std::vector<T> parse_list_to_objects(const std::string& list,
		const char delim,
		const std::function<T(const std::string& s)>& convert_func,
		int& count)
{
	auto results = std::vector<T> {};
	// TODO reserve default?

	ARCS_LOG(DEBUG2) << "Split input string by delimiter '" << delim << "'";

	count = 0;
	parse_list(list, delim,
			[&convert_func, &results, &count](const std::string& s)
			{
				++count;
				ARCS_LOG(DEBUG1) << "Parse input string part "
					<< std::setw(2) << count
					<< ": '"
					<< s
					<< "'";
				results.emplace_back(convert_func(s));
			});

	ARCS_LOG(DEBUG1) << "Parsed " << results.size()
		<< " parts of the input string";

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
 * \tparam T Parsing result type
 * \tparam R Returned object type
 */
template <typename T, typename R=T>
class InputStringParser : public StringParser
{
	/**
	 * \brief Parsing result for empty input.
	 */
	virtual void do_parse_empty() const
	{
		ARCS_LOG(DEBUG1) << "Empty parser input, return default object";
	}

	/**
	 * \brief Parsing result for non-empty input.
	 *
	 * \param[in] s String to parse
	 *
	 * \throws std::runtime_error If parsing fails
	 */
	virtual void do_parse_nonempty(const std::string& s) const
	= 0;

	/**
	 * \brief Provide the result object as a std::any.
	 *
	 * \return Parsing result wrapped in a std::any.
	 */
	virtual auto provide_object() const -> R
	= 0;

	/**
	 * \brief Internal counter for parsed units.
	 */
	mutable int count_;

	// StringParser

	std::any do_parse(const std::string& s) const final
	{
		if (s.empty())
		{
			this->do_parse_empty();
		} else
		{
			this->do_parse_nonempty(s);
		}

		return this->provide_object();
	}

protected:

	/**
	 * \brief Increase record counter.
	 *
	 * To be called in do_parse_nonempty() whenever a record is finished.
	 */
	void record_done()
	{
		++this->count_;
	}

	/**
	 * \brief Reference to counter.
	 *
	 * Can be used for output parameters like in input::parse_list_to_objects().
	 *
	 * \return Counter
	 */
	int& counter() const
	{
		return this->count_;
	}

public:

	/**
	 * \brief Result type provided by this parser.
	 */
	using result_type = T;

	/**
	 * \brief Default constructor.
	 */
	InputStringParser()
		: count_ { 0 }
	{
		// empty
	}

	/**
	 * \brief Total number of syntactic units seen in the last call of parse().
	 *
	 * \return Total number of syntactic units parsed.
	 */
	int total_records_parsed() const
	{
		return this->counter();
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
	 * \brief Internal parsing result.
	 */
	mutable DBAR result_;

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

	void do_parse_empty() const final;

	void do_parse_nonempty(const std::string& s) const final;

	DBAR provide_object() const final;

public:

	/**
	 * \brief Default constructor.
	 */
	DBARParser();
};


/**
 * \brief Type for list of ARCS values.
 */
using ChecksumValuesType = std::vector<uint32_t>;


/**
 * \brief Compatibility wrapper for a list of ARCS values.
 */
class ChecksumValuesSource final : public ChecksumSource
{
	ChecksumValuesType reference_source_;

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
	 * \brief Converting constructor for ChecksumValuesType.
	 *
	 * \param[in] values Checksum values
	 */
	ChecksumValuesSource(const ChecksumValuesType& values);

	/**
	 * \brief Default constructor.
	 */
	ChecksumValuesSource();

	/**
	 * \brief Checksum values.
	 *
	 * \return The checksum values as parsed.
	 */
	ChecksumValuesType values() const;

	using ChecksumSource::ChecksumSource;
	using ChecksumSource::operator=;
};


/**
 * \brief Parser for a list of ARCS values.
 *
 * Accepts a comma-separated list of 32 bit hexadecimal values as input.
 * Does not support blocks, i.e. all input values are considered as part of
 * block 0. The class is therefore only suitable to represent a single sequence
 * of contiguous tracks of a single album.
 */
class ChecksumValuesParser final :
			public InputStringParser<ChecksumValuesType, ChecksumValuesSource>
{
	/**
	 * \brief Internal store for parsed values.
	 */
	mutable ChecksumValuesSource values_;

	std::string start_message() const final;

	void do_parse_nonempty(const std::string& s) const final;

	ChecksumValuesSource provide_object() const final;

public:

	/**
	 * \brief Default constructor.
	 */
	ChecksumValuesParser();
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

