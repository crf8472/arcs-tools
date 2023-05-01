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

/**
 * \brief Holds data and can be printed.
 */
class Result
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	inline virtual ~Result() noexcept = default;

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
	inline virtual ~ResultList() noexcept = default;

	void append(std::unique_ptr<Result> r);

private:

	void do_print(std::ostream& o) const final;

	std::vector<std::unique_ptr<Result>> results_;
};



/**
 * \brief Hold a list of objects and print them in the order of appearance.
 */
template <typename ...Args>
class ResultObject : public Result
{
public:

	using Tuple = std::tuple<Args...>;

	inline explicit ResultObject(Args&&... args)
		: t_ { std::make_tuple(std::forward<Args&&>(args)...) }
	{
		// empty
	}

	virtual ~ResultObject() noexcept = default;

	inline void join(ResultObject&& r)
	{
		t_ = std::tuple_cat(t_, r.object());
	}

	inline const Tuple& object() const
	{
		return t_;
	}

private:

	inline virtual void do_print(std::ostream& o) const final
	{
		std::apply(
			[&o](const Args&... elements)
			{
				((o << elements), ...);
			},
			object()
		);
	}

	Tuple t_;
};

} // namespace arcsapp

#endif

