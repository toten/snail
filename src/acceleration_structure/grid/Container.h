#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <vector>
#include "concurrent_vector.h"

template <class T>
class List : public std::vector<T> {};

template <class T>
class TBB_List : public tbb::concurrent_vector<T> {};

#endif