
template <typename SUID>
limiters_repository<SUID>::limiters_repository()
    : global_limit_(0), session_limit_(0), suid_limit_(0)
    , global_limiter_(factory_.make_limiter(0, "UNINITIALIZED_GLOBAL_LIMITER"))
{
}
template <typename SUID>
void limiters_repository<SUID>::init_factory(composite_limiter_factory::type t)
{
    factory_.init(t);
}

template <typename SUID>
void limiters_repository<SUID>::global_limit(size_t limit, const std::string name)
{
    global_limit_ = limit;
    global_limiter_ = factory_.make_limiter(global_limit_, name);
}

template <typename SUID>
void limiters_repository<SUID>::session_limit(size_t limit)
{
    session_limit_ = limit;
}

template <typename SUID>
void limiters_repository<SUID>::suid_limit(size_t limit)
{
    suid_limit_ = limit;
}

template <typename SUID>
limiter limiters_repository<SUID>::global_limiter()
{
    return global_limiter_;
}

template <typename SUID>
limiter limiters_repository<SUID>::session_limiter(const std::string name)
{
   return factory_.make_limiter(session_limit_, name);
}

template <typename SUID>
composite_limiter
limiters_repository<SUID>::make_limiter(const std::string name, const std::string sn_name)
{
    composite_limiter limiter = factory_.make_composite_limiter(name);
    if(session_limit_)
        limiter.add(session_limiter(sn_name));
    if(global_limit_)
        limiter.add(global_limiter());
    return limiter;
}
template <typename SUID>
void limiters_repository<SUID>::upgrade_limiter_with(const suid& id, composite_limiter& lim)
{
    if(suid_limit_ == 0)
        return;

    limiter holder = factory_.make_limiter(suid_limit_, "suid_" + boost::lexical_cast<std::string>(id));
    limiter_impl_wptr wlim(compat::dynamic_pointer_cast<limiter::impl>(holder.get()));

    boost::mutex::scoped_lock lock(mtx_);
    std::pair<iterator, bool> res = storage_.insert(std::make_pair(id, wlim));

    // This is very tricky part:
    // If limiter for 'id' has already existed we get iterator to it (wlim was not inserted).
    // But at this moment for real object of limiter can be called destructor.
    // In this destructor will be called cleanup_hook() but it's will blocked
    // on the mutex.
    // So although iterator point to object that has already was in storage
    // if we can't lock() it it means that this object is deleting and
    // we must use value from 'holder' and reinsert it into to the storage.
    // And in release_by() we should cleanup storage only if weak_ptr is expired

    limiter_impl_ptr ptr(res.first->second.lock());
    if(ptr)
    {
        if(res.second) // the new limiter was inserted
        {
            holder.get()->cleanup(boost::bind(&limiters_repository::release_by, this, id));
            holder.acquire(lim.used());
            lim.add(holder);
        }
        else // limiter has already was in storage, just upgrade it with used memory
        {
            limiter l(ptr);
            l.acquire(lim.used());
            lim.add(l);
        }
    }
    else
    {
        // It's about tricky prat
        storage_.erase(id);
        storage_.insert(std::make_pair(id, wlim));
        holder.get()->cleanup(boost::bind(&limiters_repository::release_by, this, id));
        holder.acquire(lim.used());
        lim.add(holder);
    }
}

template <typename SUID>
size_t limiters_repository<SUID>::suid_storage_size()
{
    boost::mutex::scoped_lock lock(mtx_);
    return storage_.size();
}

template <typename SUID>
void limiters_repository<SUID>::release_by(const suid& id)
{
    boost::mutex::scoped_lock lock(mtx_);
    iterator it = storage_.find(id);
    if(it != storage_.end() && it->second.expired())
        storage_.erase(it);
}
