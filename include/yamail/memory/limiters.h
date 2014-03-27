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
    // !! Each derived class should be friend to limiters_repository<T>
    template<class T> friend class limiters_repository;

protected:
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
    // because in limiters_repository works with weak_ptr<derrived::impl>
    compat::shared_ptr<impl>& get_impl();

protected:
    // Used in derived classes
    // !! Each derived class should has the same constructor,
    // it is used in limiters_repository
    limiter(compat::shared_ptr<impl> impl);

public:

    void acquire(size_t n) throw(limiter_exhausted);
    void release(size_t n) _noexcept;

    size_t limit() const;
    size_t available() const;
    const std::string& name() const;

private:
    compat::shared_ptr<impl> impl_;
};

/*
 * class basic_limiter:
 *  simple limiter without any synchronization
 *  might be used with composite_strict_limiter or single thread code
 */
class basic_limiter : public limiter
{
    template<class T> friend class limiters_repository;

    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;
        size_t available() const;

    private:
        size_t available_;
    };

private:
    explicit basic_limiter(boost::shared_ptr<impl> impl);

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
    template<class T> friend class limiters_repository;

    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;
        size_t available() const;

    private:
        size_t available_;
        mutable boost::mutex mtx_;
    };

private:
    explicit strict_limiter(boost::shared_ptr<impl> impl);

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
    template<class T> friend class limiters_repository;

    class impl : public limiter::impl
    {
    public:
        impl(size_t limit, const std::string& name);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;
        size_t available() const;

    private:
        boost::atomic<int64_t> available_;
    };

private:
    explicit fuzzy_limiter(boost::shared_ptr<impl> impl);

public:
    explicit fuzzy_limiter(size_t limit, const std::string& name = std::string());
};

/*
 * Base class for composite limiters
 * presents pimpl idiom and provides base class for implementation.
 *
 * NOTE: this class should not be used directly for creating limiters,
 * it is only provide general realization for derived classes.
 *
 * NOTE: this class do not provide virtual methods so do not try to overload
 * this function, you need to overload only pure virtual methods of impl class.
 *
 * NOTE: Class do not provide any synchronization
 */
template <typename Limiter>
class composite_limiter
{
public:
    // !! Each derived class should define next types
    typedef Limiter limiter;
    typedef std::vector<Limiter> container;

protected:
    class impl : public boost::noncopyable
    {
    public:
        impl(const std::string& name);
        virtual ~impl();

        virtual void add(const Limiter& limiter);

        virtual void acquire(size_t n) throw(limiter_exhausted);
        virtual void release(size_t n) _noexcept;

        const std::string& name() const
        { return name_; }

        const container& limiters() const
        { return storage_; }

        size_t used() const
        { return used_; }

    private:
        container storage_;
        std::string name_;
        size_t used_;
    };

public:
    composite_limiter(compat::shared_ptr<impl> impl)
        :impl_(impl) {}

    void acquire(size_t n) throw(limiter_exhausted)
    { impl_->acquire(n); }

    void release(size_t n) _noexcept
    { impl_->release(n); }

    void add(const Limiter& limiter)
    { impl_->add(limiter); }

    const std::string& name() const
    { return impl_->name(); }

    const container& limiters() const
    { return impl_->limiters(); }

    size_t used() const
    { return impl_->used(); }

private:
    compat::shared_ptr<impl> impl_;
};

template <typename Limiter>
composite_limiter<Limiter>::impl::impl(const std::string& name)
    :name_(name), used_(0)
{
    // reserver memory for 3 limiter: global, suid, session
    storage_.reserve(3);
}

template <typename Limiter>
composite_limiter<Limiter>::impl::~impl()
{
}

template <typename Limiter>
void composite_limiter<Limiter>::impl::add(const Limiter& limiter)
{
    storage_.push_back(limiter);
}

template <typename Limiter>
void composite_limiter<Limiter>::impl::acquire(size_t n) throw(limiter_exhausted)
{
    typedef typename container::iterator iterator;
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

        throw limiter_exhausted(name() + "/" + ex.what());
    }
    used_ += n;
}

template <typename Limiter>
void composite_limiter<Limiter>::impl::release(size_t n) _noexcept
{
    typedef typename container::iterator iterator;
    iterator it = storage_.begin();
    iterator end = storage_.end();
    for(; it != end; it++)
        (*it).release(n);
    used_ -= n;
}

/*
 * class composite_fuzzy_limiter:
 *  this limiter can combine a group of limiters,
 *  provides synchronization at each limiter separately
 *
 *  NOTE: it is possible that in multithread code this limiter
 *  can acquire a little more than was set by limit
 *
 *  NOTE: Not thread safe in meaning to work with storage
 */
class composite_fuzzy_limiter: public composite_limiter<fuzzy_limiter>
{
    typedef composite_limiter<fuzzy_limiter> base_t;

public:
    typedef base_t::limiter limiter;
    typedef base_t::container container;

    explicit composite_fuzzy_limiter(const std::string& name = std::string())
        :base_t(compat::make_shared<base_t::impl>(name))
    {}
};


/*
 * class composite_strict_limiter:
 *  this limiter can combine a group of limiters,
 *  provides synchronization for all limiters simultaneously
 *
 *  NOTE: Thread safe
 */
class composite_strict_limiter: public composite_limiter<basic_limiter>
{
private:
    typedef composite_limiter<basic_limiter> base_t;

    class impl : public base_t::impl
    {
    public:
        impl(const std::string& name);

        void add(const basic_limiter& limiter);

        void acquire(size_t n) throw(limiter_exhausted);
        void release(size_t n) _noexcept;

    private:
        static boost::mutex global_composite_limiter_mutex_;
    };

public:
    typedef base_t::limiter limiter;
    typedef base_t::container container;

    explicit composite_strict_limiter(const std::string& name = std::string());
};

YAMAIL_NS_MEMORY_END
YAMAIL_NS_END

#endif // _YAMAIL_MEMORY_ALLOCATOR_H_
