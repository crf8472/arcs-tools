#ifndef ARCSTOOLS_SAFE_CAST_HPP_
#define ARCSTOOLS_SAFE_CAST_HPP_

/**
 * \file
 *
 * \brief
 */

#include <limits>                  // for numeric_limits
#include <stdexcept>               // for overflow_error, underflow_error
#include <type_traits>             // for is_signed, is_unsigned

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace service
{

// F+T signed or F+T unsigned
template <typename F, typename T,
          typename = std::enable_if_t<
              std::is_signed_v<F> == std::is_signed_v<T>>>
struct SafeCastImpl
{
	static T do_cast(F number)
	{
		// since F exceeds T check value on both ends
		if constexpr (sizeof(F) > sizeof(T))
		{
			if (number > std::numeric_limits<T>::max())
			{
				throw std::overflow_error(
						"Value is too large for target type");
			}

			if (number < std::numeric_limits<T>::min())
			{
				throw std::underflow_error(
						"Value is too small for target type");
			}
		}

		// no check required since F cannot exceed T
		return static_cast<T>(number);
	}
};

// signed F, unsigned T
template <typename F, typename T>
struct SafeCastImpl<F, T,
	std::enable_if_t<std::is_signed_v<F> && std::is_unsigned_v<T>>>
{
	static T do_cast(F number)
	{
		if (number < 0)
		{
			throw std::underflow_error(
					"Negative value cannot fit in unsigned type");
		}

		if constexpr (sizeof(F) > sizeof(T))
		{
			if (number > static_cast<F>(std::numeric_limits<T>::max()))
			{
				throw std::overflow_error(
						"Value is too large for target type");
			}
		}

		return static_cast<T>(number);
	}
};

// unsigned F, signed T
template <typename F, typename T>
struct SafeCastImpl<F, T,
          std::enable_if_t< std::is_unsigned_v<F> && std::is_signed_v<T>>>
{
	static T do_cast(F number)
	{
		if constexpr (sizeof(F) > sizeof(T))
		{
			if (number > static_cast<F>(std::numeric_limits<T>::max()))
			{
				throw std::overflow_error("Value is too large for target type");
			}
		}

		// negative values not possible for unsigned F
		return static_cast<T>(number);
	}
};


/**
 * \brief Perform a safe cast from numerical type f to numerical type T.
 *
 * \tparam F Input type to cast from
 * \tparam T Output type to cast to
 *
 * \param[in] number Numerical value to cast
 *
 * \throw overflow_error If \c number is too large for type \c T
 * \throw overflow_error If \c number is too small for type \c T
 */
template <typename T, typename F>
T safe_cast(F number)
{
	return SafeCastImpl<F, T>::do_cast(number);
}

} // namespace service
} // namespace v_1_0_0
} // namespace arcsapp

#endif

