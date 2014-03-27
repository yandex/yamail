#include <yamail/memory/limiters.h>

YAMAIL_FQNS_MEMORY_BEGIN

/* ****************************************************************************
 * limiter
 */

limiter::limiter(compat::shared_ptr<impl> impl):impl_(impl)
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

size_t limiter::available() const
{
    return impl_->available();
}

const std::string& limiter::name() const
{
    return impl_->name();
}

compat::shared_ptr<limiter::impl>& limiter::get_impl()
{
    return impl_;
}

limiter::impl::impl(size_t limit, const std::string& name)
: limit_(limit)
, name_(name)
{}

limiter::impl::~impl()
{
    if(cleanup_hook_) cleanup_hook_();
}

size_t limiter::impl::limit() const
{
    return limit_;
}

const std::string& limiter::impl::name() const
{
    return name_;
}

void limiter::impl::cleanup(boost::function<void()> hook)
{
    cleanup_hook_ = hook;
}

void limiter::impl::throw_exhausted() const throw(limiter_exhausted)
{
    throw limiter_exhausted(name());
}

/* ****************************************************************************
 * basic_limiter
 */

basic_limiter::basic_limiter(boost::shared_ptr<impl> impl)
    :limiter(impl)
{

}

basic_limiter::basic_limiter(size_t limit, const std::string& name)
    :limiter(compat::make_shared<impl>(limit, name))
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

size_t basic_limiter::impl::available() const
{
    return available_;
}

/* ****************************************************************************
 * strict_limiter
 */

strict_limiter::strict_limiter(boost::shared_ptr<impl> impl)
    :limiter(impl)
{}

strict_limiter::strict_limiter(size_t limit, const std::string& name)
    :limiter(compat::make_shared<impl>(limit, name))
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

size_t strict_limiter::impl::available() const
{
    boost::mutex::scoped_lock lock(mtx_);
    return available_;
}


/* ****************************************************************************
 * fuzzy_limiter
 */

fuzzy_limiter::fuzzy_limiter(boost::shared_ptr<impl> impl)
    :limiter(impl)
{}

fuzzy_limiter::fuzzy_limiter(size_t limit, const std::string& name)
    :limiter(compat::make_shared<impl>(limit, name))
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

size_t fuzzy_limiter::impl::available() const
{
    return available_;
}

/* ****************************************************************************
 * composite_strict_limiter
 */
boost::mutex composite_strict_limiter::impl::global_composite_limiter_mutex_;

composite_strict_limiter::composite_strict_limiter(const std::string& name)
    :base_t(compat::make_shared<impl>(name))
{
}

composite_strict_limiter::impl::impl(const std::string& name)
    : base_t::impl(name)
{
}

void composite_strict_limiter::impl::add(const basic_limiter& limiter)
{
    boost::mutex::scoped_lock lock(global_composite_limiter_mutex_);
    base_t::impl::add(limiter);
}

void composite_strict_limiter::impl::acquire(size_t n) throw(limiter_exhausted)
{
    boost::mutex::scoped_lock lock(global_composite_limiter_mutex_);
    base_t::impl::acquire(n);
}

void composite_strict_limiter::impl::release(size_t n) _noexcept
{
    boost::mutex::scoped_lock lock(global_composite_limiter_mutex_);
    base_t::impl::release(n);
}


YAMAIL_FQNS_MEMORY_END
