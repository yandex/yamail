#ifndef _YAMAIL_MEMORY_LIMITERS_H_
#define _YAMAIL_MEMORY_LIMITERS_H_
#include <yamail/config.h>
#include <yamail/memory/namespace.h>
#include <yamail/compat/shared_ptr.h>

#include <stdexcept>
#include <vector>

#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>


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
 * Holder for limiter implementation
 * Presents pimpl idiom and provides base class for implementation.
 */
class limiter
{
    template<class T> friend class limiters_repository;

public:
    class impl : public boost::noncopyable
    {
    public:
        impl(size_t limit, const std::string& name);
        virtual ~impl();

        virtual void acquire(size_t) throw(limiter_exhausted) = 0;
        virtual void release(size_t) _noexcept = 0;
        virtual size_t available() const = 0;

        size_t limit() const;
        const std::string& name() const;
        void cleanup(boost::function<void()> hook);

    protected:
        void throw_exhausted() const throw(limiter_exhausted);

    private:
        const size_t limit_;
        const std::string name_;

        // Used for cleanup weak_ptr<impl> in limiters_repository
        boost::function<void()> cleanup_hook_;
    };

private:
    // Used in limiters_repository to make weak_ptr<impl>
    // NOTE: It is not possible return directly weak_ptr<impl>
    // because in limiters_repository works with weak_ptr<derrived>
    compat::shared_ptr<impl>& get();

public:
    explicit limiter(compat::shared_ptr<impl> impl);

    void acquire(size_t n) throw(limiter_exhausted);
    void release(size_t n) _noexcept;

    size_t limit() const;
    size_t available() const;
    const std::string& name() const;

private:
    compat::shared_ptr<impl> impl_;
};
template <typename Impl>
limiter make_limiter(size_t limit, const std::string& name = std::string())
{
    return limiter(compat::make_shared<Impl>(limit, name));
}

/*
 * class unlimited_limiter:
 *  and it's all.
 */
class unlimited_limiter: public limiter::impl
{
public:
    unlimited_limiter(size_t limit, const std::string& name);

    void acquire(size_t n) throw (limiter_exhausted);
    void release(size_t n) _noexcept;
    size_t available() const;
};

/*
 * class basic_limiter:
 *  simple limiter without any synchronization
 *  might be used with composite_strict_limiter or single thread code
 */
class basic_limiter: public limiter::impl
{
public:
    basic_limiter(size_t limit, const std::string& name);

    void acquire(size_t n) throw (limiter_exhausted);
    void release(size_t n) _noexcept;
    size_t available() const;

private:
    size_t available_;
};

/*
 * class strict_limiter:
 *  the same as basic_limiter but provide synchronization
 *  might be used directly in multithread code
 */

class strict_limiter: public limiter::impl
{
public:
    strict_limiter(size_t limit, const std::string& name);

    void acquire(size_t n) throw (limiter_exhausted);
    void release(size_t n) _noexcept;
    size_t available() const;

private:
    size_t available_;
    mutable boost::mutex mtx_;
};

/*
 * class fuzzy_limiter:
 *  this limiter provide synchronization by atomic variable
 *
 *  NOTE: it is possible that in multithread code this limiter
 *  can acquire a little more than was set by limit
 */
class fuzzy_limiter: public limiter::impl
{
public:
    fuzzy_limiter(size_t limit, const std::string& name);

    void acquire(size_t n) throw (limiter_exhausted);
    void release(size_t n) _noexcept;
    size_t available() const;

private:
    boost::atomic<int64_t> available_;
};

/*
 * Holder for composite limiters
 * Presents pimpl idiom and provides base class for implementation.
 */
class composite_limiter
{
public:
    typedef std::vector<limiter> container;

public:
    class impl : public boost::noncopyable
    {
        struct scoped_lock : public boost::noncopyable
        {
            scoped_lock(boost::mutex* mtx):mtx_(mtx)
            { if(mtx_) mtx_->lock(); }

            ~scoped_lock() { if(mtx_) mtx_->unlock(); }

            boost::mutex* mtx_;
        };

    public:
        impl(const std::string& name, size_t reserve = 3);
        virtual ~impl();

        virtual boost::mutex* mutex();

        virtual void add(const limiter& limiter);

        virtual void acquire(size_t n) throw(limiter_exhausted);
        virtual void release(size_t n) _noexcept;

        const std::string& name() const;
        container limiters();
        size_t used();

    private:
        container storage_;
        std::string name_;
        size_t used_;
    };

public:
    explicit composite_limiter(compat::shared_ptr<impl> impl);

    void acquire(size_t n) throw(limiter_exhausted)
    { impl_->acquire(n); }

    void release(size_t n) _noexcept
    { impl_->release(n); }

    void add(const limiter& limiter)
    { impl_->add(limiter); }

    const std::string& name() const
    { return impl_->name(); }

    container limiters() const
    { return impl_->limiters(); }

    size_t used() const
    { return impl_->used(); }

private:
    compat::shared_ptr<impl> impl_;
};

template <typename Impl>
composite_limiter make_composite_limiter(const std::string& name = std::string())
{
    return composite_limiter(compat::make_shared<Impl>(name));
}

/*
 * class composite_unlimited_limiter:
 */
class composite_unlimited_limiter: public composite_limiter::impl
{
public:
    composite_unlimited_limiter(const std::string& name = std::string());

    virtual void add(const limiter& limiter);
    virtual void acquire(size_t n) throw(limiter_exhausted);
    virtual void release(size_t n) _noexcept;
};

/*
 * class composite_fuzzy_limiter:
 *  this limiter can combine a group of limiters,
 *  provides synchronization at each limiter separately
 *
 *  NOTE: it is possible that in multithread code this limiter
 *  can acquire a little more than was set by limit
 */
class composite_fuzzy_limiter: public composite_limiter::impl
{
public:
    composite_fuzzy_limiter(const std::string& name = std::string());

    boost::mutex* mutex();

private:
    mutable boost::mutex mutex_;
};

/*
 * class composite_strict_limiter:
 *  this limiter can combine a group of limiters,
 *  provides synchronization for all limiters simultaneously
 */
class composite_strict_limiter: public composite_limiter::impl
{
public:
    composite_strict_limiter(const std::string& name = std::string());

    boost::mutex* mutex();

private:
    static boost::mutex global_composite_limiter_mutex_;
};

limiter make_unlimited_limiter(size_t limit, const std::string& name = std::string());
limiter make_basic_limiter(size_t limit, const std::string& name = std::string());
limiter make_strict_limiter(size_t limit, const std::string& name = std::string());
limiter make_fuzzy_limiter(size_t limit, const std::string& name = std::string());

composite_limiter make_composite_unlimited_limiter(const std::string& name = std::string());
composite_limiter make_composite_fuzzy_limiter(const std::string& name = std::string());
composite_limiter make_composite_strict_limiter(const std::string& name = std::string());

YAMAIL_NS_MEMORY_END
YAMAIL_NS_END

#endif // _YAMAIL_MEMORY_ALLOCATOR_H_
