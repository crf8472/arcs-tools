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

#include <cstdint>                // for uint32_t
#include <tuple>                  // for tuple, make_tuple
#include <type_traits>            // for underlying_type_t

namespace arcsapp
{
inline namespace v_1_0_0
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
 * \return Underlying type of \p e.
 */
template <typename E>
inline constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

} // namespace details


/**
 * \brief Abstract base class for formatters.
 *
 * Each subclass will provide a function \p format() that accepts exactly the
 * parameters from the template parameter pack as const references.
 *
 * \tparam T    Formatting result type
 * \tparam Args Parameters for formatting
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
	virtual ~Layout() = default;

	/**
	 * \brief Format objects.
	 *
	 * \param[in] t Tuple of the objects to format
	 *
	 * \return Formatted result
	 */
	T format(InputTuple t) const
	{
		this->assertions(t);
		return this->do_format(t);
	}

	/**
	 * \brief Format objects.
	 *
	 * \param[in] args The objects to format
	 *
	 * \return Formatted result
	 */
	T format(const Args&... args) const
	{
		return this->format(std::make_tuple(args...));
	}

protected:

	virtual void assertions(InputTuple) const
	{
		/* empty */
	};

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
class Flags final
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] flags Initial internal state
	 */
	explicit Flags(const uint32_t flags);

	/**
	 * \brief Default Constructor.
	 *
	 * Initializes every flag to FALSE.
	 */
	Flags()
		: Flags(0)
	{
		/* empty */
	};

	/**
	 * \brief Set the specified flag to TRUE.
	 *
	 * \param[in] idx Index to set
	 */
	void set(const int idx);

	/**
	 * \brief Set the specified flag to FALSE.
	 *
	 * \param[in] idx Index to set
	 */
	void unset(const int idx);

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


// TODO Remove this type in favor of Flags. Only used in tools-table
// /**
//  * \brief A set of flags.
//  */
// template <typename T, typename S>
// class Flags2 final
// {
// public:
//
// 	/**
// 	 * \brief Type for flags.
// 	 *
// 	 * Is an unsigned numeric type.
// 	 */
// 	using type = S;
//
// 	/**
// 	 * \brief TRUE iff value for parameter \p t is TRUE, otherwise FALSE.
// 	 *
// 	 * \param[in] t  Input value to check flag value for
// 	 *
// 	 * \return TRUE iff \p t has flag value TRUE, otherwise FALSE.
// 	 */
// 	bool operator() (const T t) const
// 	{
// 		return flags_ & (1 << std::underlying_type_t<T>(t));
// 	}
//
// 	/**
// 	 * \brief Set a flag for input value \p t.
// 	 *
// 	 * \param[in] t     Input to set value for
// 	 * \param[in] value Value to be set for \p t
// 	 */
// 	void set(const T t, const bool value)
// 	{
// 		flags_ |= (value << std::underlying_type_t<T>(t));
// 	}
//
// private:
//
// 	/**
// 	 * \brief Internal flags.
// 	 */
// 	type flags_;
// };


/**
 * \brief Flags with type KEY as key.
 */
template<typename KEY>
class FlagStore final
{
public:

	/**
	 * \brief Construct with individual flags.
	 *
	 * \param[in] flags Flags to use
	 */
	explicit FlagStore(const uint32_t flags)
		: flags_ { flags }
	{
		/* empty */
	}

	/**
	 * \brief Default constructor.
	 *
	 * Default-initializes internal flags.
	 */
	FlagStore()
		: flags_ {/*default*/}
	{
		/* empty */
	}

	/**
	 * \brief Set the specified flag to TRUE.
	 *
	 * \param[in] idx Index to set
	 */
	void set(const KEY key)
	{
		flags_.set(array_index(key));
	}

	/**
	 * \brief Set the specified flag to FALSE.
	 *
	 * \param[in] idx Index to set
	 */
	void unset(const KEY key)
	{
		flags_.unset(array_index(key));
	}

	/**
	 * \brief Set value of flag \c key to \c value.
	 *
	 * Equivalent to set().
	 *
	 * \param[in] key   Key of the flag to set
	 * \param[in] value New value
	 */
	void set_flag(const KEY key, const bool value)
	{
		flags_.set_flag(array_index(key), value);
	}

	/**
	 * \brief Set value of flag \c key to \c value.
	 *
	 * Equivalent to set_flag().
	 *
	 * \param[in] key   Key of the flag to set
	 * \param[in] value New value
	 */
	void set(const KEY key, const bool value)
	{
		this->set_flag(key, value);
	}

	/**
	 * \brief Get value of flag \c key.
	 *
	 * \param[in] key Key of the flag to get
	 *
	 * \return Value of flag \c key
	 */
	bool flag(const KEY key) const
	{
		return flags_.flag(array_index(key));
	}

	/**
	 * \brief Get value of flag \c key.
	 *
	 * \param[in] key Key of the flag to get
	 *
	 * \return Value of flag \c key
	 */
	bool operator() (const KEY key) const
	{
		return this->flag(key);
	}

private:

	/**
	 * \brief Turn \c key to an array index.
	 *
	 * \return Array index
	 */
	auto array_index(const KEY key) const ->
		decltype( details::to_underlying<KEY>(key) )
	{
		return details::to_underlying<KEY>(key);
	}

	/**
	 * \brief Internal flag store.
	 */
	Flags flags_;
};


/**
 * \brief Layout with boolean flags.
 *
 * \tparam KEY   Key type for flags
 * \tparam T
 * \tparam Args
 */
template <typename KEY, typename T, typename ...Args>
class LayoutWithFlags : Layout<T, Args...>
{
public:

	/**
	 * \brief Return TRUE if layout has property \c key.
	 *
	 * \param[in] key Property to test for
	 *
	 * \return TRUE if this instance has property \c key
	 */
	bool has_property(const KEY key) const
	{
		return flag_state_.flag(key);
	}

	/**
	 * \brief Set or unset property \c key.
	 *
	 * \param[in] key   Property to set or unset
	 * \param[in] value TRUE or FALSE
	 */
	void update_property(const KEY key, const bool value)
	{
		flag_state_.set_flag(key, value);
	}

	/**
	 * \brief Sets property \c key to TRUE.
	 *
	 * \param[in] key Property to set
	 */
	void set_property(const KEY key)
	{
		update_property(key, true);
	}

	/**
	 * \brief Sets property \c key to FALSE.
	 *
	 * \param[in] key Property to unset
	 */
	void unset_property(const KEY key)
	{
		update_property(key, false);
	}

	/**
	 * \brief Toggles property \c key.
	 *
	 * \param[in] key Property to toggle
	 */
	void toggle_property(const KEY key)
	{
		update_property(key, !has_property(key));
	}

private:

	/**
	 * \brief Internal flag state.
	 */
	FlagStore<KEY> flag_state_;
};


/**
 * \brief Provides internal settings as member.
 */
class WithInternalFlags
{
public:

	/**
	 * \brief Construct with individual flags.
	 *
	 * \param[in] flags Flags to use
	 */
	explicit WithInternalFlags(const uint32_t flags) : flags_ { flags }
		{ /* empty */ }

	/**
	 * \brief Default constructor.
	 *
	 * Initializes any internal setting with FALSE.
	 */
	WithInternalFlags() : WithInternalFlags(0) { /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WithInternalFlags() noexcept = default;

protected:

	/**
	 * \brief Access internal settings.
	 *
	 * \return Settings.
	 */
	Flags& flags() { return flags_; }

	/**
	 * \brief Access internal settings.
	 *
	 * \return Settings.
	 */
	const Flags& flags() const { return flags_; }

private:

	Flags flags_;
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

