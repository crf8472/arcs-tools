#ifndef __ARCSTOOLS_APPS_HPP__
#include "apps.hpp"
#endif

#include <map>

namespace arcsapp
{
	std::unique_ptr<ApplicationFactory::MapType> ApplicationFactory::map_;
} // namespace arcsapp

