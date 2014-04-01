#ifndef LIMITERS_REPOSITORY_H_
#define LIMITERS_REPOSITORY_H_

#include <yamail/config.h>
#include <yamail/memory/namespace.h>
#include <yamail/memory/limiters.h>
#include <yamail/memory/composite_limiter_factory.h>
#include <yamail/compat/shared_ptr.h>

#include <map>
#include <string>
#include <functional>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>

YAMAIL_FQNS_MEMORY_BEGIN

class uid
{
public:
    virtual ~uid()
    {}

    virtual bool less(const uid* /*rhs*/) const
    { return false; }

    virtual const std::string& to_string() const = 0;
};

struct less_uid: public std::binary_function<const uid*, const uid*, bool>
{
    bool operator()(const uid* lhs, const uid* rhs) const
    { return lhs->less(rhs); }
};

class string_uid: public uid
{
public:
    string_uid(const std::string& key): key_(key)
    {}

    bool less(const uid* rhs) const
    { return key_ < rhs->to_string(); }

    const std::string& to_string() const
    { return key_; }

private:
    std::string key_;
};

class limiters_repository: public boost::noncopyable
{
private:
    /*
     * Class impl is available because we are friends
     * Each single limiter must be friend to limiters_repository
     */
    typedef compat::shared_ptr<limiter::impl> limiter_impl_ptr;
    typedef compat::weak_ptr<limiter::impl> limiter_impl_wptr;

    typedef std::map<const uid*, limiter_impl_wptr, less_uid> storage;
    typedef storage::iterator iterator;

public:
    static limiters_repository& inst();

    // Initialization function
    void init_factory(composite_limiter_factory::type t);
    void global_limit(size_t limit, const std::string name = std::string("GLOBAL"));
    void session_limit(size_t limit);
    void suid_limit(size_t limit);

    // Return the global limiter
    limiter global_limiter();
    // Create single limiter with name 'name'
    limiter session_limiter(const std::string name = std::string());

    /*
     * Create composite limiter, consist of:
     *  global limiter
     *  session limiter
     *  Args:
     *   name = name of composite limiter
     *   sn_name = name of local limiter
     */
    composite_limiter make_limiter(const std::string name = std::string(), const std::string sn_name = std::string("session"));

    /*
     * Add to 'limiter' limiter associated with 'id'
     * This operation add to limiter associated with 'id'
     *  amount of used memory in 'limiter'
     *
     * throw: limiter_exhausted if during merge operation
     *  limiter associated with 'id' will be exhausted,
     *  in this case limiter associated with 'id' will not be modificated
     */
    template <typename UID, typename Key>
    void upgrade_limiter_with(const Key& id, composite_limiter& limiter);

    // For checking that all right
    size_t suid_storage_size();

private:
    limiters_repository();
    // It's cleanup hook passed to limiter associated with 'id'
    // It's can be call only in destructor of limiter to delete expired weak_ptr.
    void release_by(const uid* id);

    void upgrade_limiter_with_impl(const uid* id, composite_limiter& limiter);

private:
    boost::mutex mtx_;
    storage storage_;
    composite_limiter_factory factory_;

    size_t global_limit_;
    size_t session_limit_;
    size_t suid_limit_;

    limiter global_limiter_;
};

template <typename UID, typename Key>
void limiters_repository::upgrade_limiter_with(const Key& key, composite_limiter& limiter)
{
    upgrade_limiter_with_impl(new UID(key), limiter);
}

YAMAIL_FQNS_MEMORY_END

#endif /* LIMITERS_REPOSITORY_H_ */

