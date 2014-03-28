#ifndef COMPOSITE_LIMITER_FACTORY_HPP_
#define COMPOSITE_LIMITER_FACTORY_HPP_

#include <yamail/config.h>
#include <yamail/memory/namespace.h>
#include <yamail/memory/limiters.h>
#include <string>

YAMAIL_FQNS_MEMORY_BEGIN

// Produce proper impl instance for composite_limiter
// Produce proper limiter for concrete composite_limiter
class composite_limiter_factory
{
public:
    enum type {FUZZY, STRICT, UNLIMITED};

    void init(composite_limiter_factory::type t);

    composite_limiter make_composite_limiter(const std::string& name = std::string());
    limiter make_limiter(size_t limit, const std::string& name = std::string());

private:
    type type_;
};

YAMAIL_FQNS_MEMORY_END

#endif /* COMPOSITE_LIMITER_FACTORY_HPP_ */
