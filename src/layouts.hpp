#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#define __ARCSTOOLS_LAYOUTS_HPP__

/**
 * \file
 *
 * \brief Output layouts.
 *
 * The application generates output that is to be formatted as table containing
 * numbers and strings. This module contains table layouts for ARIds,
 * DBARTriplets and for the results of the CALC and VERIFY applications. It can
 * also be used to construct new layouts.
 */

#include <array>                  // for array
#include <cstdint>                // for uint32_t
#include <memory>                 // for unique_ptr
#include <string>                 // for string
#include <tuple>                  // for tuple, make_tuple

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksum
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>       // for DBARTriplet
#endif

namespace arcsapp
{
namespace details
{

/**
 * \brief Convert an object to its underlying value.
 *
 * Convenience function to convert typed enum values to their underlying
 * value.
 *
 * \param[in] e The value to convert
 *
 * \return Underlying type of \c e.
 */
template <typename E>
inline constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

} // namespace details


/**
 * \brief Provide easy format templates for subclasses.
 *
 * Each concrete subclass will provide a function \c format() that accepts
 * exactly the parameters from the template parameter pack as const references.
 */
template <typename T, typename ...Args>
class Layout
{
public:

	/**
	 * \brief A tuple of const-references of the input arguments.
	 */
	using InputTuple = std::tuple<const Args&...>;

	/**
	 * \brief Virtual default destructor
	 */
	inline virtual ~Layout() = default;

	/**
	 * \brief Format objects.
	 *
	 * \param[in] t Tuple of the objects to format
	 */
	inline T format(InputTuple t) const
	{
		this->assertions(t);
		return this->do_format(t);
	}

	/**
	 * \brief Format objects.
	 *
	 * Convenience for not having to turn anything explicitly into tuples.
	 *
	 * \param[in] args The objects to format
	 */
	inline T format(const Args&... args) const
	{
		return this->format(std::make_tuple(args...));
	}

protected:

	virtual void assertions(InputTuple) const { /* empty */ };

private:

	virtual T do_format(InputTuple args) const
	= 0;
};


// Commented out, but left as a note:
// One could now implement a specialization with two parameter packs:
//
//template <typename ...Settings, typename ...Args>
//class Layout<std::tuple<Settings...>, std::tuple<Args...>>
//{
//public:
//
//	using SettingsTuple = std::tuple<Settings...>;
//	using ArgsTuple     = std::tuple<Args...>;
//	using InputTuple  = std::tuple<const Args*...>;
//
//	...
//};
//
// It could then be used like:
// Layout<std::tuple<int, bool>, std::<Checksum, const char*>)


/**
 * \brief Access non-public flags.
 *
 * Provides 32 boolean states with accessors.
 */
class InternalFlags final
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] flags Initial internal state
	 */
	explicit InternalFlags(const uint32_t flags);

	/**
	 * \brief Default Constructor.
	 *
	 * Initializes every flag to FALSE.
	 */
	InternalFlags() : InternalFlags(0) { /* empty */ };

	/**
	 * \brief Set the specified flag to the specified value.
	 *
	 * \param[in] idx   Index to set
	 * \param[in] value Value to set
	 */
	void set_flag(const int idx, const bool value);

	/**
	 * \brief Return the specified flag.
	 *
	 * \param[in] idx   Index to return
	 *
	 * \return The value of the specified flag
	 */
	bool flag(const int idx) const;

	/**
	 * \brief Return true if no flags are set.
	 *
	 * \return TRUE if no flags are set
	 */
	bool no_flags() const;

	/**
	 * \brief Return TRUE if only one flag is set, otherwise FALSE
	 *
	 * \return TRUE if only one flag is set, otherwise FALSE
	 */
	bool only_one_flag() const;

	/**
	 * \brief Return TRUE if the specified flag is the only flag set
	 *
	 * \param[in] idx   Index to return
	 *
	 * \return TRUE iff the specified flag is set an all others are not
	 */
	bool only(const int idx) const;

private:

	/**
	 * \brief Implementation of the flags
	 */
	uint32_t flags_;
};


/**
 * \brief Provides internal settings as member.
 */
class WithInternalFlags
{
public:

	/**
	 * \brief Construct with individual flags.
	 */
	explicit WithInternalFlags(const uint32_t flags) : flags_ { flags }
		{ /* empty */ }

	/**
	 * \brief Default constructor.
	 *
	 * Initializes any internal setting with FALSE.
	 */
	WithInternalFlags() : WithInternalFlags(0) { /* empty */ }

protected:

	/**
	 * \brief Access internal settings.
	 *
	 * \return Settings.
	 */
	InternalFlags& flags() { return flags_; }

	/**
	 * \brief Access internal settings.
	 *
	 * \return Settings.
	 */
	const InternalFlags& flags() const { return flags_; }

private:

	InternalFlags flags_;
};


/**
 * \brief Interface for formatting Checksums.
 */
using ChecksumLayout = Layout<std::string, arcstk::Checksum, int>;


/**
 * \brief Format Checksums in hexadecimal representation.
 */
class HexLayout : protected WithInternalFlags
				, public ChecksumLayout
{
public:

	/**
	 * \brief Constructor
	 */
	HexLayout();

	/**
	 * \brief Make the base '0x' visible
	 *
	 * \param[in] base Flag for showing the base
	 */
	void set_show_base(const bool base);

	/**
	 * \brief Return TRUE if the base is shown, otherwise FALSE
	 *
	 * \return TRUE if the base is shown, otherwise FALSE
	 */
	bool shows_base() const;

	/**
	 * \brief Make the hex digits A-F uppercase
	 *
	 * \param[in] base Flag for making hex digits A-F uppercase
	 */
	void set_uppercase(const bool base);

	/**
	 * \brief Return TRUE if A-F are uppercase, otherwise FALSE
	 *
	 * \return TRUE if A-F are uppercase, otherwise FALSE
	 */
	bool is_uppercase() const;

private:

	std::string do_format(InputTuple t) const override;
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

