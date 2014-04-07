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

compat::shared_ptr<limiter::impl>& limiter::get()
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
 * unlimited_limiter
 */

unlimited_limiter::unlimited_limiter(size_t limit, const std::string& name)
    : limiter::impl(limit, name)
{}

void unlimited_limiter::acquire(size_t) throw(limiter_exhausted)
{}

void unlimited_limiter::release(size_t) _noexcept
{}

size_t unlimited_limiter::available() const
{ return 0; }

/* ****************************************************************************
 * basic_limiter
 */

basic_limiter::basic_limiter(size_t limit, const std::string& name)
    : limiter::impl(limit, name)
    , available_(limit)
{}

void basic_limiter::acquire(size_t n) throw(limiter_exhausted)
{
    if(available_ >=n)
        available_ -= n;
    else
        throw_exhausted();
}

void basic_limiter::release(size_t n) _noexcept
{
    available_ += n;
}

size_t basic_limiter::available() const
{
    return available_;
}

/* ****************************************************************************
 * strict_limiter
 */

strict_limiter::strict_limiter(size_t limit, const std::string& name)
    : limiter::impl(limit, name)
    , available_(limit)
{}

void strict_limiter::acquire(size_t n) throw(limiter_exhausted)
{
    boost::mutex::scoped_lock lock(mtx_);
    if(available_ >= n)
        available_ -= n;
    else
        throw_exhausted();
}

void strict_limiter::release(size_t n) _noexcept
{
    boost::mutex::scoped_lock lock(mtx_);
    available_ += n;
}

size_t strict_limiter::available() const
{
    boost::mutex::scoped_lock lock(mtx_);
    return available_;
}


/* ****************************************************************************
 * fuzzy_limiter
 */

fuzzy_limiter::fuzzy_limiter(size_t limit, const std::string& name)
    : limiter::impl(limit, name)
    , available_(limit)
{}

void fuzzy_limiter::acquire(size_t n) throw(limiter_exhausted)
{
    if(available_.fetch_sub(n) < static_cast<uint64_t>(n))
    {
        available_.fetch_add(n);
        throw_exhausted();
    }
}

void fuzzy_limiter::release(size_t n) _noexcept
{
    available_.fetch_add(n);
}

size_t fuzzy_limiter::available() const
{
    return available_;
}

/* ****************************************************************************
 * composite_limiter
 */

composite_limiter::composite_limiter()
    :impl_(compat::make_shared<composite_unlimited_limiter>())
{
}

composite_limiter::composite_limiter(compat::shared_ptr<impl> impl)
    :impl_(impl)
{
}

void composite_limiter::acquire(size_t n) throw(limiter_exhausted)
{ impl_->acquire(n); }

void composite_limiter::release(size_t n) _noexcept
{ impl_->release(n); }

void composite_limiter::add(const limiter& limiter)
{ impl_->add(limiter); }

const std::string& composite_limiter::name() const
{ return impl_->name(); }

composite_limiter::container composite_limiter::limiters() const
{ return impl_->limiters(); }

size_t composite_limiter::used() const
{ return impl_->used();}

composite_limiter::impl::impl(const std::string& name, size_t reserve)
    :name_(name), used_(0)
{
    // reserver memory for 3 limiter: global, suid, session
    storage_.reserve(reserve);
}

composite_limiter::impl::~impl()
{
}

boost::mutex* composite_limiter::impl::mutex()
{
    return NULL;
}

void composite_limiter::impl::add(const limiter& limiter)
{
    scoped_lock lock(mutex());
    storage_.push_back(limiter);
}

void composite_limiter::impl::acquire(size_t n) throw(limiter_exhausted)
{
    scoped_lock lock(mutex());
    if(storage_.empty())
        return;

    typedef container::iterator iterator;
    iterator it = storage_.begin();
    iterator end = storage_.end();
    try
    {
        for(; it != end; it++)
        {
            (*it).acquire(n);
        }
    }
    catch(const limiter_exhausted& ex)
    {
        iterator end = it;
        iterator it = storage_.begin();
        for(; it != end; it++)
            (*it).release(n);

        throw limiter_exhausted("limiter '" + name() + "/" + ex.what() + "' exhausted");
    }
    used_ += n;
}

void composite_limiter::impl::release(size_t n) _noexcept
{
    scoped_lock lock(mutex());
    if(storage_.empty())
        return;
    typedef container::iterator iterator;
    iterator it = storage_.begin();
    iterator end = storage_.end();
    for(; it != end; it++)
        (*it).release(n);
    used_ -= n;
}

const std::string& composite_limiter::impl::name() const
{
    return name_;
}

composite_limiter::container composite_limiter::impl::limiters()
{
    scoped_lock lock(mutex());
    return storage_;
}

size_t composite_limiter::impl::used()
{
    scoped_lock lock(mutex());
    return used_;
}

/* ****************************************************************************
 * composite_unlimited_limiter
 */
composite_unlimited_limiter::composite_unlimited_limiter(const std::string& name)
: composite_limiter::impl(name, 0)
{
}

void composite_unlimited_limiter::add(const limiter&)
{}
void composite_unlimited_limiter::acquire(size_t) throw(limiter_exhausted)
{}
void composite_unlimited_limiter::release(size_t) _noexcept
{}

/* ****************************************************************************
 * composite_fuzzy_limiter
 */
composite_fuzzy_limiter::composite_fuzzy_limiter(const std::string& name)
    : composite_limiter::impl(name)
{
}

boost::mutex* composite_fuzzy_limiter::mutex()
{
    return &mutex_;
}

/* ****************************************************************************
 * composite_strict_limiter
 */
boost::mutex composite_strict_limiter::global_composite_limiter_mutex_;

composite_strict_limiter::composite_strict_limiter(const std::string& name)
    : composite_limiter::impl(name)
{
}

boost::mutex* composite_strict_limiter::mutex()
{
    return &global_composite_limiter_mutex_;
}

/* ****************************************************************************
 * make limiter functions
 */
limiter make_unlimited_limiter(size_t limit, const std::string& name)
{
    return make_limiter<unlimited_limiter>(limit, name);
}

limiter make_basic_limiter(size_t limit, const std::string& name)
{
    return make_limiter<basic_limiter>(limit, name);
}

limiter make_strict_limiter(size_t limit, const std::string& name)
{
    return make_limiter<strict_limiter>(limit, name);
}

limiter make_fuzzy_limiter(size_t limit, const std::string& name)
{
    return make_limiter<fuzzy_limiter>(limit, name);
}

composite_limiter make_composite_unlimited_limiter(const std::string& name)
{
    return make_composite_limiter<composite_unlimited_limiter>(name);
}


composite_limiter make_composite_fuzzy_limiter(const std::string& name)
{
    return make_composite_limiter<composite_fuzzy_limiter>(name);
}

composite_limiter make_composite_strict_limiter(const std::string& name)
{
    return make_composite_limiter<composite_strict_limiter>(name);
}


YAMAIL_FQNS_MEMORY_END
