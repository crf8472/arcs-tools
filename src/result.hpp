#ifndef __ARCSTOOLS_RESULT_HPP__
#define __ARCSTOOLS_RESULT_HPP__

/**
 * \file
 *
 * \brief Result interface.
 */

#include <ostream>     // for ostream
#include <memory>      // for unique_ptr
#include <tuple>       // for tuple, tuple_cat, make_tuple, apply
#include <utility>     // for forward
#include <vector>      // for vector


namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief Holds data and can be printed.
 */
class Result
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Result() noexcept = default;

	/**
	 * \brief Print the result to a specified stream.
	 *
	 * \param[in] o The stream to print the result to
	 */
	void print(std::ostream& o) const;

private:

	virtual void do_print(std::ostream& o) const
	= 0;
};


/**
 * \brief Print a result object via <<.
 *
 * \param[in] o       The stream to print the result to
 * \param[in] result  The result object to print
 */
std::ostream& operator << (std::ostream& o, const Result& result);


/**
 * \brief A printable list of results
 */
class ResultList final : public Result
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	~ResultList() noexcept = default;

	/**
	 * \brief Append a result to the list of results.
	 *
	 * \param[in] result An additional result to be appended
	 */
	void append(std::unique_ptr<Result> result);

private:

	void do_print(std::ostream& o) const final;

	/**
	 * \brief Internal list of results
	 */
	std::vector<std::unique_ptr<Result>> results_;
};



/**
 * \brief Hold a list of objects and print them in the order of appearance.
 */
template <typename ...Args>
class ResultObject : public Result
{
public:

	/**
	 * \brief Tuple of the argument types.
	 */
	using Tuple = std::tuple<Args...>;

	/**
	 * \brief Constructor.
	 *
	 * \param[in] args Arguments
	 */
	explicit ResultObject(Args&&... args)
		: t_ { std::make_tuple(std::forward<Args&&>(args)...) }
	{
		// empty
	}

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ResultObject() noexcept = default;

	/**
	 * \brief Join another ResultObject to the list of arguments.
	 */
	void join(ResultObject&& r)
	{
		t_ = std::tuple_cat(t_, r.object());
	}

	/**
	 * \brief Return the tuple of arguments.
	 */
	const Tuple& object() const
	{
		return t_;
	}

private:

	/**
	 * \brief Print the internal result tuple.
	 *
	 * The tuple members are printed in the order they occurr in the tuple.
	 * Each tuple member is printed using operator '<<'.
	 */
	void do_print(std::ostream& o) const final
	{
		std::apply(
			[&o](const Args&... elements)
			{
				((o << elements), ...);
			},
			object()
		);
	}

	/**
	 * \brief Internal list of arguments.
	 */
	Tuple t_;
};


} // namespace v_1_0_0
} // namespace arcsapp

#endif

