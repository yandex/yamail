#ifndef _YAMAIL_LOG_TYPED_PREDEFINED_H_
#define _YAMAIL_LOG_TYPED_PREDEFINED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>
#include <yamail/log/detail/process_name.h>

#include <yamail/compat/chrono.h>

#include <time.h> // timespec and co.

#include <boost/thread.hpp>

#include <boost/log/detail/thread_id.hpp>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace typed {

namespace detail {

// time_point formatter helper
template <typename TimePoint>
class time_wrapper
{
public:
  time_wrapper (TimePoint const& tp) : tp_ (tp) {}

  template <typename CharT, typename Traits>
  std::basic_ostream<CharT,Traits>&
  output (std::basic_ostream<CharT,Traits>& os) const
  {
  	namespace xx = YAMAIL_FQNS_COMPAT::chrono;
  	os << xx::time_fmt (xx::local, "%Y-%m-%d %H:%M:%S") << 
	    xx::time_point_cast<
	        YAMAIL_FQNS_COMPAT::chrono::system_clock::duration
	    > (tp_);
  	return os;
  }

private:
  TimePoint tp_;
};

template <typename CharT, typename Traits, typename TimePoint>
inline std::basic_ostream<CharT,Traits>&
operator<< (std::basic_ostream<CharT,Traits>& os, 
    time_wrapper<TimePoint> const& wrapper)
{
	return wrapper.output (os);
}

template <typename TimePoint>
time_wrapper<TimePoint>
make_time_wrapper (TimePoint const& tp)
{
	return time_wrapper<TimePoint> (tp);
}

class time_attr_helper 
{
private:
  template <typename C, typename Tr, typename A,
      typename Clock, typename SysClock>
  typename attr<C,Tr,A>::type 
  from_time_t (time_t time, long nanoseconds = 0L) const
  {
    // typedef Clock clock_type;

    static const typename Clock::time_point    dst_now = Clock::now ();
    static const typename SysClock::time_point sys_now = SysClock::now ();

    return basic_make_attr<C,Tr,A> (arg_time,
      make_time_wrapper (
        SysClock::from_time_t (time) 
            - sys_now + dst_now 
            + YAMAIL_FQNS_COMPAT::chrono::nanoseconds (nanoseconds)
      )
    );
  }

public:
  template <typename C, typename Tr, typename A,
      typename Clock, typename Duration>
  inline typename attr<C,Tr,A>::type 
  operator() (YAMAIL_FQNS_COMPAT::chrono::time_point<Clock, Duration> time)
  const
  {
    return basic_make_attr<C,Tr,A> (arg_time, make_time_wrapper (time));
  }

  template <typename C, typename Tr, typename A, 
      typename Clock, typename SysClock>
  inline typename attr<C,Tr,A>::type 
  operator() (time_t time, long nanoseconds = 0L) const
  {
  	return from_time_t<C,Tr,A,Clock,SysClock> (time, nanoseconds);
  }

  template <typename C, typename Tr, typename A> 
  inline typename attr<C,Tr,A>::type 
  operator() (time_t time, long nanoseconds = 0L) const
  {
#if 0
    return from_time_t<
                C, Tr, A
              , YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock
              , YAMAIL_FQNS_COMPAT::chrono::system_clock
     > (time, nanoseconds);
#else
		return basic_make_attr<C,Tr,A> (arg_time,
		  make_time_wrapper (
		    YAMAIL_FQNS_COMPAT::chrono::system_clock::from_time_t (time) 
		      + YAMAIL_FQNS_COMPAT::chrono::nanoseconds (nanoseconds)
		  )
    );
#endif
  }

  template <typename C, typename Tr, typename A> 
  inline typename attr<C,Tr,A>::type 
  operator() (struct timeval const& tv) const
  {
    return operator ()<C,Tr,A> (tv.tv_sec, 1000L * tv.tv_usec);
  }

  template <typename C, typename Tr, typename A> 
  inline typename attr<C,Tr,A>::type 
  operator() (struct timespec const& ts) const
  {
    return operator()<C,Tr,A> (ts.tv_sec, ts.tv_nsec);
  }

  template <typename C, typename Tr, typename A> 
  inline typename attr<C,Tr,A>::type 
  operator() () const
  {
    return operator()<C,Tr,A> (time (0));
  }


}; // time_attr_helper

struct pid_attr_helper 
{
	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
	operator() () const
	{
		return basic_make_attr<C,Tr,A> (arg_pid, ::getpid ());
  }

	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
	operator() (::pid_t pid) const
	{
		return basic_make_attr<C,Tr,A> (arg_pid, pid);
  }
};

struct ppid_attr_helper
{
	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
  operator() () const
  {
		return basic_make_attr<C,Tr,A> (arg_ppid, ::getppid ());
  }

	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
  operator() (::pid_t ppid) const
  {
		return basic_make_attr<C,Tr,A> (arg_ppid, ppid);
  }
};

struct tid_attr_helper
{
	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
	operator() () const
	{
		return basic_make_attr<C,Tr,A> (arg_tid, 
		    boost::log::aux::this_thread::get_id ());
  }

	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
	operator() (boost::thread::id tid) const
	{
		return basic_make_attr<C,Tr,A> (arg_tid, tid);
  }
};

struct process_name_attr_helper
{
	// TODO: cache process name
	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
	operator() () const
	{
		return basic_make_attr<C,Tr,A> (arg_process,  
		    YAMAIL_FQNS_LOG::detail::get_process_name ());
  }

	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
	operator() (std::string const& process_name) const
	{
		return basic_make_attr<C,Tr,A> (arg_process, process_name);
  }
};

struct priority_attr_helper
{
	template <typename C, typename Tr, typename A>
	typename attr<C,Tr,A>::type 
  operator() (priority_enum prio) const
  {
  	return basic_make_attr<C,Tr,A> (arg_priority, prio);
  }
};

template <typename T> struct is_predefined { static const bool value = false; };

template<> struct is_predefined<time_attr_helper> 
{ static const bool value = true; };

template<> struct is_predefined<pid_attr_helper> 
{ static const bool value = true; };

template<> struct is_predefined<ppid_attr_helper> 
{ static const bool value = true; };

template<> struct is_predefined<tid_attr_helper> 
{ static const bool value = true; };

template<> struct is_predefined<process_name_attr_helper> 
{ static const bool value = true; };

template<> struct is_predefined<priority_attr_helper> 
{ static const bool value = true; };
} // namespace detail


template <typename C, typename Tr, typename A, typename P>
inline typename boost::enable_if_c<
    detail::is_predefined<P>::value
  , basic_attributes_map<C,Tr,A>&
>::type
operator<< (basic_attributes_map<C,Tr,A>& map, P const& predefined)
{
  return map << predefined.template operator()<C,Tr,A> ();
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
