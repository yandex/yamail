#include <yamail/memory/limiters.h>

YAMAIL_FQNS_MEMORY_BEGIN

/* ****************************************************************************
 * limiter
 */

limiter::limiter(boost::shared_ptr<impl> impl):impl_(impl)
{}

void limiter::acquire(size_t n) throw(limiter_exhausted)
{
    impl_->acquire(n);
}

void limiter::release(size_t n) _noexcept
{
    impl_->release(n);
}

size_t limiter::limit() const
{
    return impl_->limit();
}

const std::string& limiter::name() const
{
    return impl_->name();
}

boost::shared_ptr<limiter::impl> limiter::get_impl()
{
    return impl_;
}

limiter::impl::impl(size_t limit, const std::string& name)
: limit_(limit)
, name_(name)
{}

limiter::impl::~impl()
{}

size_t limiter::impl::limit() const
{
    return limit_;
}

const std::string& limiter::impl::name() const
{
    return name_;
}

void limiter::impl::throw_exhausted() const throw(limiter_exhausted)
{
    throw limiter_exhausted(name());
}

/* ****************************************************************************
 * basic_limiter
 */

basic_limiter::basic_limiter(size_t limit, const std::string& name)
    :limiter(boost::shared_ptr<impl>(new impl(limit, name)))
{}

basic_limiter::impl::impl(size_t limit, const std::string& name)
    : limiter::impl(limit, name)
    , available_(limit)
{}

void basic_limiter::impl::acquire(size_t n) throw(limiter_exhausted)
{
    if(available_ >=n)
        available_ -= n;
    else
        throw_exhausted();
}

void basic_limiter::impl::release(size_t n) _noexcept
{
    available_ += n;
}

/* ****************************************************************************
 * strict_limiter
 */

strict_limiter::strict_limiter(size_t limit, const std::string& name)
    :limiter(boost::shared_ptr<impl>(new impl(limit, name)))
{}

strict_limiter::impl::impl(size_t limit, const std::string& name)
    : limiter::impl(limit, name)
    , available_(limit)
{}

void strict_limiter::impl::acquire(size_t n) throw(limiter_exhausted)
{
    boost::mutex::scoped_lock lock(mtx_);
    if(available_ >= n)
        available_ -= n;
    else
        throw_exhausted();
}

void strict_limiter::impl::release(size_t n) _noexcept
{
    boost::mutex::scoped_lock lock(mtx_);
    available_ += n;
}

/* ****************************************************************************
 * fuzzy_limiter
 */

fuzzy_limiter::fuzzy_limiter(size_t limit, const std::string& name)
    :limiter(boost::shared_ptr<impl>(new impl(limit, name)))
{}

fuzzy_limiter::impl::impl(size_t limit, const std::string& name)
    : limiter::impl(limit, name)
    , available_(limit)
{}

void fuzzy_limiter::impl::acquire(size_t n) throw(limiter_exhausted)
{
    if(available_.fetch_sub(n) < static_cast<int64_t>(n))
    {
        available_.fetch_add(n);
        throw_exhausted();
    }
}

void fuzzy_limiter::impl::release(size_t n) _noexcept
{
    available_.fetch_add(n);
}

/* ****************************************************************************
 * composite_fuzzy_limiter
 */

composite_fuzzy_limiter::composite_fuzzy_limiter(const std::string& name)
    : limiter(boost::shared_ptr<impl>(new impl(name)))
{ }

void composite_fuzzy_limiter::add(const fuzzy_limiter& limiter)
{
    dynamic_cast<impl*>(get_impl().get())->add(limiter);
}

composite_fuzzy_limiter::impl::impl(const std::string& name):
    limiter::impl(0, name)
{
}

void composite_fuzzy_limiter::impl::add(const fuzzy_limiter& limiter)
{
    storage_.push_back(limiter);
}

void composite_fuzzy_limiter::impl::acquire(size_t n) throw(limiter_exhausted)
{
    storage_t::iterator it = storage_.begin();
    storage_t::iterator end = storage_.end();
    try
    {
        for(; it != end; it++)
        {
            (*it).acquire(n);
        }
    }
    catch(const limiter_exhausted& ex)
    {
        storage_t::iterator end = it;
        storage_t::iterator it = storage_.begin();
        for(; it != end; it++)
        (*it).release(n);

        throw limiter_exhausted(name() + "/" + ex.what());
    }
}

void composite_fuzzy_limiter::impl::release(size_t n) _noexcept
{
    storage_t::iterator it = storage_.begin();
    storage_t::iterator end = storage_.end();
    for(; it != end; it++)
    (*it).release(n);
}

composite_strict_limiter::composite_strict_limiter(const std::string& name)
    : limiter(boost::shared_ptr<impl>(new impl(name)))
{
}

/* ****************************************************************************
 * composite_strict_limiter
 */

boost::mutex composite_strict_limiter::impl::global_composite_limiter_mutex_;

void composite_strict_limiter::add(const basic_limiter& limiter)
{
    dynamic_cast<impl*>(get_impl().get())->add(limiter);
}

composite_strict_limiter::impl::impl(const std::string& name)
    : limiter::impl(0, name)
{
}

void composite_strict_limiter::impl::add(const basic_limiter& limiter)
{
    storage_.push_back(limiter);
}

void composite_strict_limiter::impl::acquire(size_t n) throw(limiter_exhausted)
{
    boost::mutex::scoped_lock lock(global_composite_limiter_mutex_);

    storage_t::iterator it = storage_.begin();
    storage_t::iterator end = storage_.end();
    try
    {
        for(; it != end; it++)
        {
            (*it).acquire(n);
        }
    }
    catch(const limiter_exhausted& ex)
    {
        storage_t::iterator end = it;
        storage_t::iterator it = storage_.begin();
        for(; it != end; it++)
            (*it).release(n);

        throw limiter_exhausted(name() + "/" + ex.what());
    }
}

void composite_strict_limiter::impl::release(size_t n) _noexcept
{
    boost::mutex::scoped_lock lock(global_composite_limiter_mutex_);

    storage_t::iterator it = storage_.begin();
    storage_t::iterator end = storage_.end();
    for(; it != end; it++)
        (*it).release(n);
}


YAMAIL_FQNS_MEMORYY_END
