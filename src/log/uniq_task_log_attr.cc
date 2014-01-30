#ifndef YPLATFORM_WITHOUT_BOOST_LOG
#include <yamail/log/uniq_task_log_attr.h>
#include <boost/thread.hpp>
#include <stdlib.h>
#include <netdb.h>
#include <iomanip>
#include <sstream>
#include <sys/syscall.h> // gettid
#include <sys/types.h>

YAMAIL_NS_BEGIN
YAMAIL_NS_LOG_BEGIN

namespace {

unsigned char get_local_ip_addr()
{
  hostent* h = gethostent();
  if (!h)
    return 0;
  return h->h_addr[h->h_length - 1];
}

const unsigned char LOCAL_IP_ADDRESS = get_local_ip_addr();

const char code_table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwx";
const size_t code_table_size (sizeof(code_table)/sizeof(*code_table) - 1);

const size_t single_divisor (code_table_size);
const size_t double_divisor (single_divisor * code_table_size);
const size_t triple_divisor (double_divisor * code_table_size);

class number_generator
{
public:
  typedef unsigned int number_type;

  static number_generator& instance ()
  {
    static number_generator rnd;
    return rnd;
  }

  number_type operator() ()
  {
    boost::lock_guard<boost::mutex> lck (mux_);
    return ++counter_;
  }

private:
  number_generator ()
  {
    counter_ = 0;
  }

private:
  number_type counter_;
  boost::mutex mux_;
};

} // namespace anon

#if BOOST_VERSION < 104600
uniq_task_attr::uniq_task_attr (const std::string& ini)
#else
uniq_task_attr::impl::impl (std::string& val, const std::string& ini)
#endif
{
  if (!ini.empty ()) {
    value_ = ini;
#if BOOST_VERSION >= 104600
    val = value_;
#endif
    return;
  }
  pid_t tid = 0;
#if defined(SYS_gettid)
  tid = static_cast<pid_t> (::syscall (SYS_gettid));
#endif
  time_t now = time(0);
  struct tm lt;
  localtime_r(&now, &lt);

  number_generator::number_type generated_value = number_generator::instance () ();

  value_.reserve(12);
  value_ += code_table[lt.tm_sec];
  value_ += code_table[lt.tm_min];
  value_ += code_table[(lt.tm_hour + lt.tm_mday + lt.tm_mon) % code_table_size];
  value_ += code_table[(generated_value                 ) % code_table_size];
  value_ += code_table[(generated_value / single_divisor) % code_table_size];
  value_ += code_table[(generated_value / double_divisor) % code_table_size];
  value_ += code_table[(generated_value / triple_divisor) % code_table_size];
  value_ += code_table[getpid() % code_table_size];
  value_ += code_table[tid % code_table_size];
  value_ += code_table[(reinterpret_cast<unsigned long long> (this)) % 
    code_table_size];
  value_ += code_table[(reinterpret_cast<unsigned long long> (this) + 
      code_table_size / 2) % code_table_size];
  value_ += code_table[LOCAL_IP_ADDRESS % code_table_size];
#if BOOST_VERSION >= 104600
  val = value_;
#endif
}

YAMAIL_NS_LOG_END
YAMAIL_NS_END
#endif
