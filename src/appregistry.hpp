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
#include <iterator>  // for iterator
#include <map>       // for map
#include <memory>    // for unique_ptr
#include <set>       // for set
#include <string>    // for string
#include <utility>   // for make_pair

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

namespace arcsapp
{

namespace details
{

/**
 * \brief Instantiate Application
 *
 * \tparam T    The type to instantiate
 * \tparam Args The constructor arguments
 */
template <class T, typename... Args>
std::unique_ptr<Application> instantiateApplication(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace details


/**
 * \brief Returns TRUE if \c callstr is equal to or ends with \c name
 *
 * \param[in] name    Application name requested
 * \param[in] callstr Call string
 *
 * \return TRUE if \c callstr is equal to or ends with \c name, otherwise FALSE
 */
inline bool matches_name(const std::string &name, const std::string &callstr)
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
	 * \brief Return first match for a key name with \c callstr
	 *
	 * \param[in] callstr Name of an application type or call string
	 *
	 * \return Instance of the first application type matching the input
	 */
	static std::unique_ptr<Application> lookup(std::string const &callstr)
	{
		for (const auto& entry : *get_map())
		{
			if (matches_name(entry.first, callstr))
			{
				return instantiate(entry.first, entry.second);
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
	static std::unique_ptr<Application> instantiate(std::string const &name)
	{
		MapType::iterator it = get_map()->find(name);

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
		std::set<std::string> names;
		std::transform(get_map()->begin(), get_map()->end(),
				std::inserter(names, names.begin()), RetrieveName());
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
	 * \return Instance returned by \c create
	 */
	static std::unique_ptr<Application> instantiate(std::string const &,
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
};


/**
 * \brief Register an Application type
 *
 * \tparam T The type to register
 */
template <class T>
class RegisterApplicationType : ApplicationFactory //TODO SFINAE exclude types
{
public:

	/**
	 * \brief Register a type by a specific name
	 *
	 * \param[in] name The name to register the application type
	 */
	RegisterApplicationType(std::string const& name)
	{
		get_map()->insert(
				std::make_pair(name, &details::instantiateApplication<T>));
	}
};

} // namespace arcsapp

#endif

