#ifndef YPLATFORM_WITHOUT_BOOST_LOG
#include <yamail/log/rotate_text_file_backend.h>

#include <list>
#include <memory>
#include <string>
#include <locale>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#if !defined(BOOST_LOG_NO_THREADS) && !defined(BOOST_SPIRIT_THREADSAFE)
#define BOOST_SPIRIT_THREADSAFE
#endif

#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/list_hook.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/compatibility/cpp_c_headers/cctype>
#include <boost/compatibility/cpp_c_headers/cwctype>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/compatibility/cpp_c_headers/cstdio>
#include <boost/compatibility/cpp_c_headers/cstdlib>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/log/detail/snprintf.hpp>
#include <boost/log/detail/singleton.hpp>
#include <boost/log/attributes/time_traits.hpp>
#if BOOST_VERSION < 104600
#include <boost/log/detail/throw_exception.hpp>
#endif

#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#endif

YAMAIL_NS_BEGIN
YAMAIL_NS_LOG_BEGIN

template <typename CharT>
struct file_char_traits;

template< >
struct file_char_traits< char >
{
  typedef char char_type;
  static const char_type newline = '\n';
};

template< >
struct file_char_traits< wchar_t >
{
  typedef wchar_t char_type;
  static const char_type newline = L'\n';
};


//! Sink implementation data
template< typename CharT >
struct rotate_basic_text_file_backend< CharT >::implementation
{
    //! File open mode
    std::ios_base::openmode m_FileOpenMode;

    //! Current file name
    path_type m_FilePath;
    //! File stream
    boost::filesystem::basic_ofstream< CharT > m_File;

    bool m_AutoFlush;

    implementation(bool auto_flush, const path_type& file_path) :
        m_FileOpenMode(std::ios_base::app),
        m_FilePath(file_path),
        m_AutoFlush(auto_flush)
    {
    }
};

//! Constructor. No streams attached to the constructed backend, auto flush feature disabled.
template< typename CharT >
rotate_basic_text_file_backend< CharT >::rotate_basic_text_file_backend()
{
    construct(boost::log::aux::empty_arg_list());
}

//! Destructor
template< typename CharT >
rotate_basic_text_file_backend< CharT >::~rotate_basic_text_file_backend()
{
    try
    {
        // Attempt to put the temporary file into storage
        log_rotate_manager::instance().unregister_sink(this);
        if (m_pImpl->m_File.is_open())
            rotate();
    }
    catch (...)
    {
    }

    delete m_pImpl;
}

//! Constructor implementation
template< typename CharT >
void rotate_basic_text_file_backend< CharT >::construct(
    path_type const& file_name,
    std::ios_base::openmode mode,
    bool auto_flush)
{
    m_pImpl = new implementation(auto_flush, file_name);
    set_open_mode(mode);
    log_rotate_manager::instance().register_sink(this);
}

//! Sets the flag to automatically flush buffers of all attached streams after each log record
template< typename CharT >
void rotate_basic_text_file_backend< CharT >::auto_flush(bool f)
{
    m_pImpl->m_AutoFlush = f;
}

//! The method writes the message to the sink
#if BOOST_VERSION < 105400
template< typename CharT >
void rotate_basic_text_file_backend< CharT >::do_consume(
    record_type const& /*record*/, target_string_type const& formatted_message)
#else
template< typename CharT >
void rotate_basic_text_file_backend< CharT >::consume(
    boost::log::record_view const&, string_type const& formatted_message)
#endif
{
    lock_t lock(mux_);
#if BOOST_VERSION < 105400
    typedef file_char_traits< typename target_string_type::value_type > traits_t;
#else
    typedef file_char_traits< char_type > traits_t;
#endif
    if (!m_pImpl->m_File.is_open())
    {
        boost::filesystem::create_directories(m_pImpl->m_FilePath.parent_path());
        m_pImpl->m_File.open(m_pImpl->m_FilePath, m_pImpl->m_FileOpenMode);
        if (!m_pImpl->m_File.is_open())
        {
#if BOOST_VERSION < 104600
          boost::log::aux::throw_exception(boost::filesystem::basic_filesystem_error< path_type >(
              "failed to open file for writing",
              m_pImpl->m_FilePath,
              boost::system::error_code(boost::system::errc::io_error, boost::system::get_generic_category()))
          );
#else
            boost::throw_exception(boost::system::system_error (
                boost::system::error_code(boost::system::errc::io_error, boost::system::get_generic_category())
            ));
#endif
        }
    }
    m_pImpl->m_File.write(formatted_message.data(), static_cast< std::streamsize >(formatted_message.size()));
    m_pImpl->m_File.put(traits_t::newline);

    if (m_pImpl->m_AutoFlush)
        m_pImpl->m_File.flush();
}

//! The method sets file name
template< typename CharT >
void rotate_basic_text_file_backend< CharT >::set_file_name_internal(path_type const& file_name)
{
  m_pImpl->m_FilePath = file_name;
}

//! The method rotates the file
template< typename CharT >
void rotate_basic_text_file_backend< CharT >::rotate()
{
    lock_t lock(mux_);
    m_pImpl->m_File.flush();
    m_pImpl->m_File.close();
    m_pImpl->m_File.clear();
}

//! The method sets the file open mode
template< typename CharT >
void rotate_basic_text_file_backend< CharT >::set_open_mode(std::ios_base::openmode mode)
{
    mode |= std::ios_base::out;
    mode &= ~std::ios_base::in;
    if ((mode & (std::ios_base::trunc | std::ios_base::app)) == 0)
        mode |= std::ios_base::app;
    m_pImpl->m_FileOpenMode = mode;
}

//! Explicitly instantiate sink backend implementations
#ifdef BOOST_LOG_USE_CHAR
template class rotate_basic_text_file_backend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class rotate_basic_text_file_backend< wchar_t >;
#endif

YAMAIL_NS_LOG_END
YAMAIL_NS_END

#endif
