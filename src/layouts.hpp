#ifndef ARCSTOOLS_LAYOUTS_HPP_
#define ARCSTOOLS_LAYOUTS_HPP_
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

#include <algorithm>              // for transform
#include <cstdint>                // for uint32_t
#include <map>                    // for map
#include <set>                    // for set
#include <string>                 // for string
#include <tuple>                  // for tuple, make_tuple
#include <type_traits>            // for underlying_type_t


namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief Type for a sequence of boolean flags.
 */
using flags_t = uint32_t; // TODO this should be a bitfield or vector<bool>


/**
 * \brief Empty string instance.
 */
static const auto EmptyString = std::string {/*empty*/};


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


/**
 * \brief Turn a numeric enum class type and a boolean to an arithmetic flag.
 *
 * Convenience function to convert typed enum values and their boolean values
 * to a concrete flag value.
 *
 * \param[in] type  The type to convert
 * \param[in] value The value to convert
 *
 * \return A numerical flag
 */
template<typename E>
inline constexpr flags_t flag_operand(const E type, const bool value)
{
	return static_cast<flags_t>(value) << to_underlying(type);
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
	 * \brief Value for flags_t to represent all flags are TRUE.
	 */
	static constexpr flags_t ALL_TRUE  = 0xFFFFFFFF;

	/**
	 * \brief Value for flags_t to represent all flags are FALSE.
	 */
	static constexpr flags_t ALL_FALSE = 0x00000000;

	/**
	 * \brief Constructor.
	 *
	 * \param[in] flags Initial internal state
	 */
	explicit Flags(const flags_t flags);

	/**
	 * \brief Default Constructor.
	 *
	 * Initializes every flag to FALSE.
	 */
	Flags();

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
	flags_t flags_;
};


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
	 * \param[in] flags Initial internal state
	 */
	explicit FlagStore(const flags_t flags)
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
		: flags_ { /*default*/ }
	{
		/* empty */
	}

	/**
	 * \brief Set the specified flag to TRUE.
	 *
	 * \param[in] key Key to set to TRUE
	 */
	void set(const KEY key)
	{
		flags_.set(array_index(key));
	}

	/**
	 * \brief Set the specified flag to FALSE.
	 *
	 * \param[in] key Key to set to FALSE
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

	/**
	 * \brief Return true if no flags are set.
	 *
	 * \return TRUE if no flags are set
	 */
	bool no_flags() const
	{
		return flags_.no_flags();
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
 * \brief Base class with boolean properties.
 */
template<typename KEY>
class PropertyStore
{
public:

	/**
	 * \brief Constructor with flags.
	 *
	 * \param[in] flags Initial internal state
	 */
	explicit PropertyStore(const flags_t flags)
		: flag_store_ { flags }
	{
		// empty
	}

	/**
	 * \brief Default constructor.
	 */
	PropertyStore()
		: flag_store_ { /*default*/ }
	{
		// empty
	}

	/**
	 * \brief Default destructor.
	 */
	virtual ~PropertyStore() noexcept = default;

	/**
	 * \brief Return TRUE if layout has property \c key.
	 *
	 * \param[in] key Property to test for
	 *
	 * \return TRUE if this instance has property \c key
	 */
	bool has_property(const KEY key) const
	{
		return flag_store_.flag(key);
	}

	/**
	 * \brief Set or unset property \c key.
	 *
	 * \param[in] key   Property to set or unset
	 * \param[in] value TRUE or FALSE
	 */
	void update_property(const KEY key, const bool value)
	{
		flag_store_.set_flag(key, value);
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

	/**
	 * \brief Returns TRUE if no properties are set.
	 *
	 * \return TRUE if no properties are set, otherwise FALSE
	 */
	bool no_properties() const
	{
		return flag_store_.no_flags();
	}

private:

	/**
	 * \brief Internal flag state.
	 */
	FlagStore<KEY> flag_store_;
};


/**
 * \brief Template for associating keys with labels.
 *
 * \tparam KEY Key type
 */
template <typename KEY>
class LabelStore
{
public:

	/**
	 * \brief Internal key type.
	 */
	using key_t = KEY;

	/**
	 * \brief Internal store type.
	 */
	using store_t = std::map<const key_t, std::string>;

	/**
	 * \brief Default constructor.
	 */
	LabelStore()
		: labels_ { /*default*/ }
	{
		// empty
	}

	/**
	 * \brief Constructor to set all labels.
	 *
	 * \param[in] labels List of key-label pairs for initialization
	 */
	explicit LabelStore(store_t labels)
		: labels_ { std::move(labels) }
	{
		// empty
	}

	/**
	 * \brief Default destructor.
	 */
	virtual ~LabelStore() noexcept = default;

	/**
	 * \brief Set all labels.
	 *
	 * \param[in] labels All labels
	 */
	void set_labels(const store_t& labels)
	{
		labels_ = labels;
	}

	/**
	 * \brief Set a label for \c key.
	 *
	 * \param[in] key   The key to set a label for
	 * \param[in] label The label to set for \c key
	 */
	void set_label(const key_t key, const std::string& label)
	{
		labels_.insert_or_assign(key, label);
	}

	/**
	 * \brief Get the label for \c key.
	 *
	 * \param[in] key The key to get the label for
	 *
	 * \return The label for \c key
	 */
	const std::string& label(const key_t key) const
	{
		const auto label_ptr { labels_.find(key) };

		using std::cend;
		if (cend(labels_) == label_ptr)
		{
			return EmptyString;
		}

		return label_ptr->second;
	}

private:

	/**
	 * \brief Internal association of KEYs with labels.
	 */
	store_t labels_;
};


/**
 * \brief Get the set of keys as flags for a corresponding FlagStore.
 *
 * \tparam S Store type
 *
 * \param[in] labels Instance of a store_t to get keys from
 *
 * \return FlagStore flags which are TRUE excatly for the existing KEYs
 */
template<typename S>
flags_t existing_flags(const S& labels)
{
	using std::cbegin;
	using std::cend;

	auto flags = flags_t { Flags::ALL_FALSE };

	std::for_each(cbegin(labels), cend(labels),
		[&flags](const auto& pair)
		{
			return flags |= details::flag_operand(pair.first, true);
		}
	);

	return flags;
}

} // namespace v_1_0_0
} // namespace arcsapp

#endif

