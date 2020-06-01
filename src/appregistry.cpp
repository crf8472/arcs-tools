#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"
#endif

#include <map>

namespace arcsapp
{
	std::unique_ptr<ApplicationFactory::MapType> ApplicationFactory::map_;
} // namespace arcsapp

