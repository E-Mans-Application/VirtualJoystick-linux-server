#ifndef _IO_EXCEPTION_HPP_
#define _IO_EXCEPTION_HPP_

#include <exception>

class IOException : public std::exception {
  private:
    int _errno;

  public:
    IOException(int errno_);
    virtual const char *what() const noexcept override;
};

#endif