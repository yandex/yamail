#include <yamail/memory/composite_limiter_factory.h>

YAMAIL_FQNS_MEMORY_BEGIN

void composite_limiter_factory::init(composite_limiter_factory::type t)
{
    type_ = t;
}

composite_limiter composite_limiter_factory::make_composite_limiter(const std::string& name)
{
    switch(type_)
    {
    case FUZZY:
        return make_composite_fuzzy_limiter(name);
    case STRICT:
        return make_composite_strict_limiter(name);
    default:
        break;
    }
    return make_composite_unlimited_limiter(name);
}


limiter composite_limiter_factory::make_limiter(size_t limit, const std::string& name)
{
    switch(type_)
    {
    case FUZZY:
        return make_fuzzy_limiter(limit, name);
    case STRICT:
        return make_basic_limiter(limit, name); // it's not a mistake
    default:
        break;
    }
    return make_unlimited_limiter(limit, name); // hide warning
}

YAMAIL_FQNS_MEMORY_END



