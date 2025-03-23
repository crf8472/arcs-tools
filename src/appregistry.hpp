#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#define __ARCSTOOLS_APPREGISTRY_HPP__

/**
 * \file
 *
 * \brief Interface for application registry.
 *
 * Provides the facility to associate Application types with names such that
 * instances of the types can be requested by this name.
 */

#include <algorithm> // for transform
#include <iterator>  // for inserter
#include <map>       // for map
#include <memory>    // for unique_ptr, make_unique
#include <set>       // for set
#include <string>    // for string
#include <utility>   // for forward, make_pair

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace details
{

/**
 * \brief Instantiate Application
 *
 * \tparam T    The type to instantiate
 * \tparam Args The constructor arguments
 *
 * \param args Argument list
 *
 * \return Application requested by input arguments
 */
template <class T, typename... Args>
inline std::unique_ptr<Application> instantiateApplication(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace details


/**
 * \brief Returns TRUE if \p callstr is equal to or ends with \p name
 *
 * \param[in] name    Application name requested
 * \param[in] callstr Call string
 *
 * \return TRUE if \p callstr is equal to or ends with \p name, otherwise FALSE
 */
inline bool matches_name(const std::string& name, const std::string& callstr)
{
	if (name.empty() or callstr.empty() or name.length() > callstr.length())
	{
		return false;
	}

	return name == callstr
		or name == callstr.substr(callstr.length() - name.length());
}


/**
 * \brief Function pointer to function returning std::unique_ptr<T>.
 *
 * \tparam T The type the function should return a unique_ptr to
 */
template <class T>
using FunctionReturning = std::unique_ptr<T>(*)();


/**
 * \brief Create instances of registered Application types
 */
class ApplicationFactory
{
public:

	using MapType = std::map<std::string, FunctionReturning<Application>>;

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ApplicationFactory() = default;

	/**
	 * \brief Return first match for a key name with \p callstr
	 *
	 * \param[in] callstr Name of an application type or call string
	 *
	 * \return Instance of the first application type matching the input
	 */
	static std::unique_ptr<Application> lookup(const std::string& callstr)
	{
		// TODO Use find on a map???
		for (const auto& [app_name, app_creator] : *get_map())
		{
			if (matches_name(app_name, callstr))
			{
				return instantiate(app_name, app_creator);
			}
		}

		return nullptr;
	}

	/**
	 * \brief Instantiate application by its exact name
	 *
	 * \param[in] name Name of an application type
	 *
	 * \return Instance of the requested application type
	 */
	static std::unique_ptr<Application> instantiate(const std::string& name)
	{
		auto it = MapType::iterator { get_map()->find(name) };

		if (it == get_map()->end())
		{
			return nullptr;
		}

		return instantiate(it->first, it->second);
	}

	/**
	 * \brief Return the set of names of available applications.
	 *
	 * \return Names of available applications.
	 */
	static std::set<std::string> registered_names()
	{
		using std::begin;
		using std::end;

		auto* fmap { get_map() };
		if (!fmap)
		{
			return std::set<std::string>{};
		}

		std::set<std::string> names{};
		std::transform(begin(*fmap), end(*fmap),
				std::inserter(names, begin(names)), RetrieveName());
		return names;
	}


protected:

	/**
	 * \brief Pointer to map for internal use in subclasses
	 *
	 * \return Internal type map
	 */
	static MapType* get_map()
	{
		if (!map_)
		{
			map_ = std::make_unique<MapType>();
		}

		return map_.get();
	}

	/**
	 * \brief Instantiate the application type with the given name.
	 *
	 * The string parameter is ignored.
	 *
	 * \param[in] create Function pointer to create the instance
	 *
	 * \return Instance returned by \p create
	 */
	static std::unique_ptr<Application> instantiate(const std::string&,
			FunctionReturning<Application> create)
	{
		return create();
	}

private:

	/**
	 * \brief Retrieve application name from an entry of map_.
	 */
	struct RetrieveName
	{
		template <typename T>
		typename T::first_type operator()(T keyValuePair) const
		{
			return keyValuePair.first;
		}
	};

	/**
	 * \brief Map associating Application types with names
	 */
	static std::unique_ptr<MapType> map_;

	virtual void do_not_make_this_class_abstract() const = 0;
};


/**
 * \brief Register an Application type
 *
 * \tparam T The type to register
 */
template <class T> //TODO SFINAE exclude types
class RegisterApplicationType final : ApplicationFactory
{
	virtual void do_not_make_this_class_abstract() const final { /* empty */ }

public:

	/**
	 * \brief Register a type by a specific name
	 *
	 * \param[in] name The name to register the application type
	 */
	RegisterApplicationType(const std::string& name)
	{
		get_map()->insert(
				std::make_pair(name, &details::instantiateApplication<T>));
	}
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

