#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"
#endif

#include <memory>    // for unique_ptr

namespace arcsapp
{
inline namespace v_1_0_0
{
	std::unique_ptr<ApplicationFactory::MapType> ApplicationFactory::map_;
} // namespace v_1_0_0
} // namespace arcsapp

