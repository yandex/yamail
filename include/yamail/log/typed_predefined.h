#ifndef _YAMAIL_LOG_TYPED_PREDEFINED_H_
#define _YAMAIL_LOG_TYPED_PREDEFINED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>
#include <yamail/log/detail/process_name.h>

#include <yamail/compat/chrono.h>

#include <time.h> // timespec and co.

#include <boost/thread.hpp>

#if defined(HAVE_STD_CHRONO) && HAVE_STD_CHRONO
namespace std {
namespace chrono {

template <typename C, typename T, class Cl, class D>
std::basic_ostream<C,T>&
operator<< (std::basic_ostream<C,T>& os, time_point<Cl,D> const& tp)
{
	return os << "(time_point)";
}

} // namespace chrono
}
#endif

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace typed {

namespace detail {

class time_attr_helper 
{
private:
  template <typename Clock, typename SysClock>
  attr_type 
  from_time_t (time_t time, long nanoseconds = 0L) const
  {
    // typedef Clock clock_type;

    static const typename Clock::time_point    dst_now = Clock::now ();
    static const typename SysClock::time_point sys_now = SysClock::now ();

    return make_attr (arg_time,
        SysClock::from_time_t (time) 
            - sys_now + dst_now 
            + YAMAIL_FQNS_COMPAT::chrono::nanoseconds (nanoseconds)
    );
  }

public:
  template <typename Clock, typename Duration>
  inline attr_type
  operator() (YAMAIL_FQNS_COMPAT::chrono::time_point<Clock, Duration> time)
  const
  {
    return make_attr (arg_time, time);
  }

  template <typename Clock, typename SysClock>
  inline attr_type 
  operator() (time_t time, long nanoseconds = 0L) const
  {
  	return from_time_t<Clock, SysClock> (time, nanoseconds);
  }

  inline attr_type operator() (time_t time, long nanoseconds = 0L) const
  {
    return from_time_t<
                YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock
              , YAMAIL_FQNS_COMPAT::chrono::system_clock
     > (time, nanoseconds);
  }

  inline attr_type operator() (struct timeval const& tv) const
  {
    return (*this) (tv.tv_sec, 1000L * tv.tv_usec);
  }

  attr_type operator() (struct timespec const& ts) const
  {
    return (*this) (ts.tv_sec, ts.tv_nsec);
  }
}; // time_attr_helper

struct pid_attr_helper 
{
	attr_type operator() () const
	{
		return make_attr (arg_pid, ::getpid);
  }

	attr_type operator() (::pid_t pid) const
	{
		return make_attr (arg_pid, pid);
  }
};

struct ppid_attr_helper
{
  attr_type operator() () const
  {
		return make_attr (arg_ppid, ::getppid);
  }

  attr_type operator() (::pid_t ppid) const
  {
		return make_attr (arg_ppid, ppid);
  }
};

struct tid_attr_helper
{
	attr_type operator() (boost::thread::id tid) const
	{
		return make_attr (arg_tid, tid);
  }
};

struct process_name_attr_helper
{
	// TODO: cache process name
	attr_type operator() () const
	{
		return make_attr (arg_process, YAMAIL_FQNS_LOG::detail::get_process_name ());
  }

	attr_type operator() (std::string const& process_name) const
	{
		return make_attr (arg_process, process_name);
  }
};

struct priority_attr_helper
{
  attr_type operator() (priority_enum prio) const
  {
  	return make_attr (arg_priority, prio);
  }
};

} // namespace detail

template <typename X> struct predefined_traits
{
  static const bool value = false;
};

template <> struct predefined_traits<attributes_map>
{
	static const bool value = true;
};

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T&
>::type
operator<< (T& map, detail::time_attr_helper const& time)
{
	return map 
	    << time (YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock::now ());
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T const&
>::type
operator<< (T const& map, detail::time_attr_helper const& time)
{
	return map 
	    << time (YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock::now ());
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T&
>::type
operator<< (T& map, detail::pid_attr_helper const& pid)
{
  return map << pid ();
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T const&
>::type
operator<< (T const& map, detail::pid_attr_helper const& pid)
{
  return map << pid ();
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T&
>::type
operator<< (T& map, detail::ppid_attr_helper const& ppid)
{
  return map << ppid ();
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T const&
>::type
operator<< (T const& map, detail::ppid_attr_helper const& ppid)
{
  return map << ppid ();
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T&
>::type
operator<< (T& map, detail::tid_attr_helper const& tid)
{
  return map << tid (boost::this_thread::get_id ());
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T const&
>::type
operator<< (T const& map, detail::tid_attr_helper const& tid)
{
  return map << tid (boost::this_thread::get_id ());
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T&
>::type
operator<< (T& map, detail::process_name_attr_helper const& srv)
{
	// TODO: get process_name name somehow (from proccess name?)
  return map << srv ();
}

template <typename T>
inline typename boost::enable_if_c<
    predefined_traits<T>::value
  , T const&
>::type
operator<< (T const& map, detail::process_name_attr_helper const& srv)
{
	// TODO: get service name somehow (from proccess name?)
  return map << srv ();
}

namespace {
const detail::time_attr_helper time_attr = detail::time_attr_helper ();
const detail::pid_attr_helper pid_attr = detail::pid_attr_helper ();
const detail::ppid_attr_helper ppid_attr = detail::ppid_attr_helper ();
const detail::tid_attr_helper tid_attr = detail::tid_attr_helper ();
const detail::process_name_attr_helper process_name_attr =
    detail::process_name_attr_helper ();
const detail::priority_attr_helper priority_attr = 
    detail::priority_attr_helper ();
}


} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_PREDEFINED_H_
