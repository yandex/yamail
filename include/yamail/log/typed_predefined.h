#ifndef _YAMAIL_LOG_TYPED_PREDEFINED_H_
#define _YAMAIL_LOG_TYPED_PREDEFINED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>
#include <yamail/log/detail/process_name.h>

#include <yamail/compat/chrono.h>

#include <ctime> // timespec and co.

#include <boost/version.hpp>
#include <boost/thread.hpp>

// this_thread::get_id
#if BOOST_VERSION >= 105500
# include <boost/log/detail/thread_id.hpp>
#else
# include <boost/thread.hpp>
#endif

#if !defined(HAVE_STD_CHRONO) || !HAVE_STD_CHRONO
# include <boost/tuple/tuple.hpp>
# include <boost/static_assert.hpp>
#endif

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace typed {

namespace detail {

// time_point formatter helper
template <typename Derived, typename TimePoint>
class time_wrapper_base
{
public:
  time_wrapper_base (TimePoint const& tp) : tp_ (tp) {}

  template <typename CharT, typename Traits>
  std::basic_ostream<CharT,Traits>&
  output (std::basic_ostream<CharT,Traits>& os) const
  {
  	namespace cchrono = YAMAIL_FQNS_COMPAT::chrono;

  	cchrono::system_clock::time_point tmp =
	    cchrono::time_point_cast<cchrono::system_clock::duration> (tp_);

    const static CharT fmt_time[] = {
    	'%', 'Y', '-', '%', 'm', '-', '%', 'd', ' ',
    	'%', 'H', '-', '%', 'M', '-', '%', 'S', '\0'
    };

    const static CharT fmt_zone[] = { '%', 'z', '\0' };

    const static CharT timezone[] = 
      { '\t', 't', 'i', 'm', 'e', 'z', 'o', 'n', 'e', '\0' };

#if 0
  	os 
  	  << time_fmt (cchrono::local, fmt_time) << tmp
  	  << timezone
  	  << time_fmt (cchrono::local, fmt_zone) << tmp
    ;
#else
		static_cast<Derived const&> (*this).
		  print_time_point (os, tmp, timezone, fmt_time, fmt_zone);
#endif
  	return os;
  }

private:
  TimePoint tp_;
};

template <typename TimePoint> struct time_wrapper;

#if defined(HAVE_STD_CHRONO) && HAVE_STD_CHRONO
template <typename Clock, typename Duration>
struct time_wrapper<std::chrono::time_point<Clock,Duration> >
  : time_wrapper_base<
        time_wrapper<std::chrono::time_point<Clock,Duration> >,
        std::chrono::time_point<Clock,Duration>
    >
{
	typedef time_wrapper_base <
	    time_wrapper<std::chrono::time_point<Clock,Duration> >,
	    std::chrono::time_point<Clock,Duration>
	> time_wrapper_base_;

	time_wrapper (std::chrono::time_point<Clock,Duration> const& tp) 
	  : time_wrapper_base_ (tp) 
	{
	}

	template <typename TimePoint, typename CharT, typename Traits>
	void print_time_point (std::basic_ostream<CharT,Traits>& os,
	    TimePoint const& tp, CharT const* timezone_str, 
	    CharT const* time_str, CharT const* zone_str) const
	{
		namespace cchrono = YAMAIL_FQNS_COMPAT::chrono;
		os << time_fmt (cchrono::local, time_str) << tp
		   << timezone_str
		   << time_fmt (cchrono::local, zone_str) << tp;
  }
};
#else // using boost::chrono below

// convert time_point<system_clock> into tm
// See
// http://stackoverflow.com/questions/16773285/how-to-convert-stdchronotime-point-to-stdtm-without-using-time-t


// Returns year/month/day triple in civil calendar
// Preconditions:  z is number of days since 1970-01-01 and is in the range:
//                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-719468].
template <class Int>
_constexpr
boost::tuple<Int, unsigned, unsigned>
civil_from_days(Int z) _noexcept
{
    BOOST_STATIC_ASSERT_MSG(std::numeric_limits<unsigned>::digits >= 18,
             "This algorithm has not been ported to a 16 bit unsigned integer");
    BOOST_STATIC_ASSERT_MSG(std::numeric_limits<Int>::digits >= 20,
             "This algorithm has not been ported to a 16 bit signed integer");
    z += 719468;
    const Int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
    const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    const Int y = static_cast<Int>(yoe) + era * 400;
    const unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
    const unsigned mp = (5*doy + 2)/153;                                   // [0, 11]
    const unsigned d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
    const unsigned m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
    return boost::tuple<Int, unsigned, unsigned>(y + (m <= 2), m, d);
}


template <class To, class Rep, class Period>
To
round_down(const compat::chrono::duration<Rep, Period>& d)
{
    To t = compat::chrono::duration_cast<To>(d);
    if (t > d)
        --t;
    return t;
}

// Returns number of days since civil 1970-01-01.  Negative values indicate
//    days prior to 1970-01-01.
// Preconditions:  y-m-d represents a date in the civil (Gregorian) calendar
//                 m is in [1, 12]
//                 d is in [1, last_day_of_month(y, m)]
//                 y is "approximately" in
//                   [numeric_limits<Int>::min()/366, numeric_limits<Int>::max()/366]
//                 Exact range of validity is:
//                 [civil_from_days(numeric_limits<Int>::min()),
//                  civil_from_days(numeric_limits<Int>::max()-719468)]
template <class Int>
_constexpr Int
days_from_civil(Int y, unsigned m, unsigned d) _noexcept
{
    BOOST_STATIC_ASSERT_MSG(std::numeric_limits<unsigned>::digits >= 18,
             "This algorithm has not been ported to a 16 bit unsigned integer");
    BOOST_STATIC_ASSERT_MSG(std::numeric_limits<Int>::digits >= 20,
             "This algorithm has not been ported to a 16 bit signed integer");
    y -= m <= 2;
    const Int era = (y >= 0 ? y : y-399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
    const unsigned doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
    const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
    return era * 146097 + static_cast<Int>(doe) - 719468;
}


template <class Int>
_constexpr unsigned weekday_from_days(Int z) _noexcept
{
    return static_cast<unsigned>(z >= -4 ? (z+4) % 7 : (z+5) % 7 + 6);
}


template <class Duration>
std::tm
make_utc_tm(compat::chrono::time_point<
    compat::chrono::system_clock, Duration> tp)
{
  using namespace boost;
  using namespace compat::chrono;

  typedef duration<int, ratio_multiply<hours::period, ratio<24> > > days;
  // t is time duration since 1970-01-01
  Duration t = tp.time_since_epoch();
  // d is days since 1970-01-01
  days d = round_down<days>(t);
  // t is now time duration since midnight of day d
  t -= d;
  // break d down into year/month/day
  int year;
  unsigned month;
  unsigned day;
  boost::tie(year, month, day) = civil_from_days(d.count());
  // start filling in the tm with calendar info
  std::tm tm = {0};
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_wday = weekday_from_days(d.count());
  tm.tm_yday = d.count() - days_from_civil(year, 1, 1);
  // Fill in the time
  tm.tm_hour = duration_cast<hours>(t).count();
  t -= hours(tm.tm_hour);
  tm.tm_min = duration_cast<minutes>(t).count();
  t -= minutes(tm.tm_min);
  tm.tm_sec = duration_cast<seconds>(t).count();
  return tm;
}


template <typename Clock, typename Duration>
struct time_wrapper<boost::chrono::time_point<Clock,Duration> >
  : time_wrapper_base<
        time_wrapper<boost::chrono::time_point<Clock,Duration> >,
        boost::chrono::time_point<Clock,Duration>
    >
{
	typedef time_wrapper_base <
	    time_wrapper<boost::chrono::time_point<Clock,Duration> >,
	    boost::chrono::time_point<Clock,Duration>
	> time_wrapper_base_;

	time_wrapper (boost::chrono::time_point<Clock,Duration> const& tp) 
	  : time_wrapper_base_ (tp) 
	{
	}

	template <typename TimePoint, typename CharT, typename Traits>
	void print_time_point (std::basic_ostream<CharT,Traits>& os,
	    TimePoint const& tp, CharT const* timezone_str, 
	    CharT const* time_str, CharT const* zone_str) const
	{
    std::tm tm = make_utc_tm (tp);

    char tmp_buf[80]; // "80 bytes is enough for everybode"

    if (0 == std::strftime (tmp_buf, sizeof (tmp_buf), time_str, &tm))
    {
      BOOST_THROW_EXCEPTION (std::runtime_error ("invalid time"));
    }

    os << tmp_buf;

    if (0 == std::strftime (tmp_buf, sizeof (tmp_buf), zone_str, &tm))
    {
      BOOST_THROW_EXCEPTION (std::runtime_error ("invalid time zone"));
    }

    os << timezone_str << tmp_buf;
#if 0
		namespace cchrono = YAMAIL_FQNS_COMPAT::chrono;
		os << cchrono::time_fmt (cchrono::timezone::local, time_str) << tp
		   << timezone_str
		   << cchrono::time_fmt (cchrono::timezone::local, zone_str) << tp;
#endif
  }
};
#endif

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
#if BOOST_VERSION >= 105500
		    boost::log::aux::this_thread::get_id ()
#else
        boost::this_thread::get_id ()
#endif
    );
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
