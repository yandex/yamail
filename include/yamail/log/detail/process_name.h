#ifndef _YAMAIL_LOG_DETAIL_PROCESS_NAME_H_
#define _YAMAIL_LOG_DETAIL_PROCESS_NAME_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <climits> // PATH_MAX
#include <boost/filesystem/path.hpp>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#if defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)

#include <cstring>
#include <mach-o/dyld.h>
#include <boost/cstdint.hpp>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace detail {

//! The function returns the current process name
std::string get_process_name()
{
  std::string buf;
  buf.resize(PATH_MAX);
  while (true)
  {
    uint32_t size = static_cast< uint32_t >(buf.size());
    if (_NSGetExecutablePath(&buf[0], &size) == 0)
    {
        buf.resize(std::strlen(&buf[0]));
        break;
    }

    buf.resize(size);
  }

  return boost::filesystem::path(buf).filename().string();
}

} // namespace detail
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#else

#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace detail {

//! The function returns the current process name
std::string get_process_name()
{
  if (filesystem::exists("/proc/self/exe"))
    return filesystem::read_symlink("/proc/self/exe").filename().string();

  if (filesystem::exists("/proc/curproc/file"))
    return filesystem::read_symlink("/proc/curproc/file").filename().string();

  if (filesystem::exists("/proc/curproc/exe"))
    return filesystem::read_symlink("/proc/curproc/exe").filename().string();

  return boost::lexical_cast< std::string >(getpid());
}

} // namespace detail
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif

#endif // _YAMAIL_LOG_DETAIL_PROCESS_NAME_H_
