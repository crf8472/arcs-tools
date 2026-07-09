#ifndef ARCSTOOLS_TOOLS_VALIDATE_HPP_
#define ARCSTOOLS_TOOLS_VALIDATE_HPP_
/**
 * \file
 *
 * \brief Validating objects.
 */

#include <functional>             // for function

#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>     // for ARCS_LOG_DEBUG, ARCS_LOG_ERROR
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief Validation tools.
 */
namespace valid
{

/**
 * \brief Encapsulate a single validation.
 *
 * \tparam Ts One or more input types of instances to validate
 */
template <typename ...Ts>
class Validate
{
	/**
	 * \brief Implements perform() without parameters.
	 */
	void perform_worker() const
	{
		ARCS_LOG(DEBUG1) << "No parameters, trivial success";
	}

	/**
	 * \brief Implements perform() with parameters.
	 */
	void perform_worker(const Ts&... ts) const
	{
		if (this->succeeds(ts...))
		{
			this->success();
			return;
		}

		this->fail();
	}

	/**
	 * \brief Implements failure of the validation operation.
	 *
	 * Logs error message and throws a std::runtime_error.
	 */
	void fail() const
	{
		ARCS_LOG(DEBUG1) << "Validate: '" << this->desc_ << "'" " failed";

		throw std::runtime_error(this->error_msg_);
	}

	/**
	 * \brief Implements success of the validation operation.
	 *
	 * Logs success message.
	 */
	void success() const noexcept
	{
		ARCS_LOG(DEBUG1) << "Validate: '" << this->desc_ << "'" " succeeded";
	}

public:

	/**
	 * \brief Type of the internal validation function.
	 */
	using validation_type = std::function<bool(const Ts&...)>;

	/**
	 * \brief Constructor.
	 *
	 * \param[in] desc      Short description of the Validation
	 * \param[in] test_func Validation function
	 * \param[in] error_msg Message on error
	 */
	Validate(std::string desc,
			const validation_type& test_func, std::string error_msg)
		: desc_      { std::move(desc) }
		, func_      { test_func }
		, error_msg_ { std::move(error_msg) }
	{
		// empty
	}

	/**
	 * \brief Perform validation.
	 *
	 * If this function does not throw, the validation is successful.
	 *
	 * \param[in] ts Objects to validate
	 */
	void perform(const Ts&... ts) const
	{
		ARCS_LOG(DEBUG1) << "Validate: " << this->desc_;

		this->perform_worker(ts...);
	}

	/**
	 * \brief Perform validation.
	 *
	 * \param[in] ts Objects to validate
	 *
	 * \return TRUE if validation succeeds, otherwise FALSE.
	 */
	bool succeeds(const Ts&... ts) const
	{
		return this->func_(ts...);
	}

private:

	/**
	 * \brief Internal short description or title.
	 */
	std::string     desc_;

	/**
	 * \brief Internal validation function.
	 */
	validation_type func_;

	/**
	 * \brief Internal error message.
	 */
	std::string     error_msg_;
};


} // namespace valid
} // namespace v_1_0_0
} // namespace arcsapp

#endif

