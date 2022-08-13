#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <vector>
#if PARALLEL
#include "concurrent_vector.h"
#endif

template <class T>
class List : public std::vector<T> {};

#if PARALLEL
template <class T>
class TBB_List : public tbb::concurrent_vector<T> {};
#endif

#endif