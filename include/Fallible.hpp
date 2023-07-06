#ifndef _FALLIBLE_HPP_
#define _FALLIBLE_HPP_

#include "IOException.hpp"

#include <cerrno>
#include <iostream>

/// This class provides method to react to errors that occurs in
/// system calls.
/// This class should be inherited.
class Fallible {
  protected:
    /// Checks the result of a system call, and throws an exception if the
    /// operation has failed.
    /// @param result The value returned by the system call
    /// @param err_value The value that would be returned in case of error
    /// @return The result of the system call, if the operation was successful.
    /// @throw Throw an `IOException` with the value of 'errno' if 'result ==
    /// err_value'
    inline int check(int result, int err_value = -1) const {
        if (result == err_value) {
            throw IOException(errno);
        }
        return result;
    }

    /// Checks the result of a system call, and prints a warning in stdout
    /// instead of throwing if the operation has failed.
    /// @return result, in any case
    /// @see check
    inline int try_opt(int result, const char *message, int err_value = -1) const noexcept {
        try {
            return check(result, err_value);
        } catch (IOException &ex) {
            std::cout << "WARNING: " << message << " failed: " << ex.what() << "\n";
            return err_value;
        }
    }
};

#endif