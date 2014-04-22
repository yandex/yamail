#ifndef _YAMAIL_CONCURRENCY_FUTURE_FUTURE_EXCEPTIONS_HPP_
#define _YAMAIL_CONCURRENCY_FUTURE_FUTURE_EXCEPTIONS_HPP_
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>

#include <stdexcept>

YAMAIL_FQNS_CONCURRENCY_BEGIN

struct broken_promise : public std::exception {
  virtual const char *what() const throw () {
    return "Broken Promise Exception";
  }
};

struct future_already_set : public std::exception {
  virtual const char *what() const throw () {
    return "Future Already Set Exception";
  }
};

struct future_cancel : public std::exception {
  virtual const char *what() const throw () {
    return "Future Canceled Exception";
  }
};

YAMAIL_FQNS_CONCURRENCY_END

#endif //_YPLATFORM_FUTURE_FUTURE_EXCEPTIONS_HPP_
