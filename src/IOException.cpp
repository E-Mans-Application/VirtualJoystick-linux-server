#include "IOException.hpp"
#include <string.h> //strerror

IOException::IOException(int errno) : _errno(errno) {}

const char *IOException::what() const noexcept { return strerror(_errno); }