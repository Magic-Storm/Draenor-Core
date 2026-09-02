#ifndef TRINITY_OPTIONAL_H
#define TRINITY_OPTIONAL_H

#include <boost/optional.hpp>

template <typename T>
using Optional = boost::optional<T>;

#endif
