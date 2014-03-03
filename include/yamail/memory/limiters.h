#ifndef _YAMAIL_MEMORY_LIMITERS_H_
#define _YAMAIL_MEMORY_LIMITERS_H_
#include <yamail/config.h>
#include <yamail/memory/namespace.h>
#include <yamail/compat/shared_ptr.h>

#include <stdexcept>
#include <vector>
#include <list>

#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>


YAMAIL_NS_BEGIN
YAMAIL_NS_MEMORY_BEGIN

namespace compat = YAMAIL_FQNS_COMPAT;

class limiter_exhausted: public std::runtime_error
{
public:
    limiter_exhausted(const std::string& name): std::runtime_error(name)
    {}

    ~limiter_exhausted() _noexcept {}
};

/*
 * Base class for limiters
 * presents pimpl idiom and provides base class for implementation.
 *
 * NOTE: this class should not be used directly for creating limiters,
 * it is only provide general realization for derived classes.
 *
 * NOTE: this class do not provide virtual methods so do not try overload
 * this function, you need to overload only pure virtual methods of impl class.
 */
class limiter
{
protected:
    class impl : public boost::noncopyable
    {
    public:
        impl(size_t limit, const std::string& name);
        virtual ~impl();

        virtual void acquire(size_t) throw(limiter_exhausted) = 0;
        virtual void release(size_t) _noexcept = 0;

        size_t limit() const;

        const std::string& name() const;

    protected:
        void throw_exhausted() const throw(limiter_exhausted);

    private:
        const size_t limit_;
        const std::string name_;
    };

public:
    limiter(boost::shared_ptr<impl> impl);

    void acquire(size_t n) throw(limiter_exhausted);
    void release(size_t n) _noexcept;

    size_t limit() const;
    const std::string& name() const;

protected:
    boost::shared_ptr<impl> get_impl();

private:
    boost::shared_ptr<impl> impl_;
};

/*
 * class basic_limiter:
 *  simple limiter without any synchronization
 *  might be used with composite_strict_limiter or single thread code
 */
class basic_limiter : public limiter
{
    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;

    private:
        size_t available_;
    };

public:
    explicit basic_limiter(size_t limit, const std::string& name = std::string());
};

/*
 * class strict_limiter:
 *  the same as basic_limiter but provide synchronization
 *  might be used directly in multithread code
 */
class strict_limiter : public limiter
{
    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;

    private:
        size_t available_;
        boost::mutex mtx_;
    };

public:
    explicit strict_limiter(size_t limit, const std::string& name = std::string());
};

/*
 * class fuzzy_limiter:
 *  this limiter provide synchronization by atomic variable
 *
 *  NOTE: it is possible that in multithread code this limiter
 *  can acquire a little more than was set by limit
 */
class fuzzy_limiter : public limiter
{
    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;

    private:
        boost::atomic<int64_t> available_;
    };

public:
    explicit fuzzy_limiter(size_t limit, const std::string& name = std::string());
};

/*
 * class composite_fuzzy_limiter:
 *  this limiter can combine a group of limiters,
 *  provides synchronization at each limiter separately
 *
 *  NOTE: it is possible that in multithread code this limiter
 *  can acquire a little more than was set by limit
 */
class composite_fuzzy_limiter : public limiter
{
    class impl : public limiter::impl
    {
        typedef std::vector<fuzzy_limiter> storage_t;
    public:
        impl(const std::string& name);

        void add(const fuzzy_limiter& limiter);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;

    private:
        storage_t storage_;
    };

public:
    explicit composite_fuzzy_limiter(const std::string& name = std::string());

    void add(const fuzzy_limiter& limiter);
};

/*
 * class composite_strict_limiter:
 *  this limiter can combine a group of limiters,
 *  provides synchronization for all limiters simultaneously
 */
class composite_strict_limiter : public limiter
{
    class impl : public limiter::impl
    {
        typedef std::vector<basic_limiter> storage_t;
    public:
        impl(const std::string& name);

        void add(const basic_limiter& limiter);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;

    private:
        static boost::mutex global_composite_limiter_mutex_;
        storage_t storage_;
    };

public:
    explicit composite_strict_limiter(const std::string& name = std::string());

    void add(const basic_limiter& limiter);
};

#if 0
// The same but in one step
class limiter
{
protected:
    class impl : public boost::noncopyable
    {
    public:
        impl(size_t limit, const std::string& name)
            : limit_(limit)
            , name_(name)
        {}

        virtual ~impl() {}

        virtual void release(size_t) _noexcept {};
        virtual void acquire(size_t) throw(limiter_exhausted)
        { throw_exhausted(); };

        size_t limit() const
        { return limit_; }

        const std::string& name() const
        { return name_; }

    protected:
        void throw_exhausted() const throw(limiter_exhausted)
        { throw limiter_exhausted(name()); }

    private:
        const size_t limit_;
        const std::string name_;
    };

public:
    limiter(boost::shared_ptr<impl> impl):impl_(impl){}

    void acquire(size_t n) throw(limiter_exhausted)
    { impl_->acquire(n); }

    void release(size_t n) _noexcept
    { impl_->release(n); }

    size_t limit() const
    { return impl_->limit(); }

    const std::string& name() const
    { return impl_->name(); }

protected:
    boost::shared_ptr<impl> get_impl()
    { return impl_; }

private:
    boost::shared_ptr<impl> impl_;
};

class basic_limiter : public limiter
{
    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name)
            : limiter::impl(limit, name)
            , available_(limit)
        {}

        void acquire(size_t n) throw(limiter_exhausted)
        {
            if(available_ >=n)
                available_ -= n;
            else
                throw_exhausted();
        }

        void release(size_t n) _noexcept
        { available_ += n; }

    private:
        size_t available_;
    };

public:
    explicit basic_limiter(size_t limit, const std::string& name = std::string())
        :limiter(boost::shared_ptr<impl>(new impl(limit, name)))
    {}
};

class strict_limiter : public limiter
{
    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name)
            : limiter::impl(limit, name)
            , available_(limit)
        {}

        void acquire(size_t n) throw(limiter_exhausted)
        {
            boost::mutex::scoped_lock lock(mtx_);
            if(available_ >= n)
                available_ -= n;
            else
                throw_exhausted();
        }

        void release(size_t n) _noexcept
        {
            boost::mutex::scoped_lock lock(mtx_);
            available_ += n;
        }

    private:
        size_t available_;
        boost::mutex mtx_;
    };

public:
    explicit strict_limiter(size_t limit, const std::string& name = std::string())
        :limiter(boost::shared_ptr<impl>(new impl(limit, name)))
    {}
};

class fuzzy_limiter : public limiter
{
    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name)
            : limiter::impl(limit, name)
            , available_(limit)
        {}

        void acquire(size_t n) throw(limiter_exhausted)
        {
            if(available_.fetch_sub(n) < static_cast<int64_t>(n))
            {
                available_.fetch_add(n);
                throw_exhausted();
            }
        }

        void release(size_t n) _noexcept
        { available_.fetch_add(n); }

    private:
        boost::atomic<int64_t> available_;
    };

public:
    explicit fuzzy_limiter(size_t limit, const std::string& name = std::string())
        :limiter(boost::shared_ptr<impl>(new impl(limit, name)))
    {}
};


class composite_fuzzy_limiter : public limiter
{
    class impl : public limiter::impl
    {
        typedef std::vector<fuzzy_limiter> storage_t;
    public:
        impl(const std::string& name): limiter::impl(0, name)
        {}

        void add(const fuzzy_limiter& limiter)
        { storage_.push_back(limiter); }

        void acquire(size_t n) throw(limiter_exhausted)
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

        void release(size_t n) _noexcept
        {
            storage_t::iterator it = storage_.begin();
            storage_t::iterator end = storage_.end();
            for(; it != end; it++)
                (*it).release(n);
        }

    private:
        storage_t storage_;
    };

public:
    explicit composite_fuzzy_limiter(const std::string& name = std::string())
        : limiter(boost::shared_ptr<impl>(new impl(name)))
    { }

    void add(const fuzzy_limiter& limiter)
    { dynamic_cast<impl*>(get_impl().get())->add(limiter); }
};

class composite_strict_limiter : public limiter
{
    class impl : public limiter::impl
    {
        typedef std::vector<basic_limiter> storage_t;
    public:
        impl(const std::string& name): limiter::impl(0, name)
        {}

        void add(const basic_limiter& limiter)
        { storage_.push_back(limiter); }

        void acquire(size_t n) throw(limiter_exhausted)
        {
            boost::mutex::scoped_lock lock(global_collector_mutex_);

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

        void release(size_t n) _noexcept
        {
            boost::mutex::scoped_lock lock(global_collector_mutex_);

            storage_t::iterator it = storage_.begin();
            storage_t::iterator end = storage_.end();
            for(; it != end; it++)
                (*it).release(n);
        }

    private:
        static boost::mutex global_collector_mutex_;
        storage_t storage_;
    };

public:
    explicit composite_strict_limiter(const std::string& name = std::string())
        : limiter(boost::shared_ptr<impl>(new impl(name)))
    { }

    void add(const basic_limiter& limiter)
    { dynamic_cast<impl*>(get_impl().get())->add(limiter); }
};
#endif

YAMAIL_NS_MEMORY_END
YAMAIL_NS_END

#endif // _YAMAIL_MEMORY_ALLOCATOR_H_
