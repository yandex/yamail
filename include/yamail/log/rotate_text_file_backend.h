#ifndef _YAMAIL_LOG_ROTATE_TEXT_FILE_BACKEND_H_
#define _YAMAIL_LOG_ROTATE_TEXT_FILE_BACKEND_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <ios>
#include <boost/limits.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function/function1.hpp>
#include <boost/date_time/special_defs.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/keywords/max_size.hpp>
#include <boost/log/keywords/min_free_space.hpp>
#include <boost/log/keywords/target.hpp>
#include <boost/log/keywords/file_name.hpp>
#include <boost/log/keywords/open_mode.hpp>
#include <boost/log/keywords/auto_flush.hpp>
#include <boost/log/keywords/rotation_size.hpp>
#include <boost/log/detail/parameter_tools.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#if BOOST_VERSION < 105400
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/universal_path.hpp>
#else
#include <boost/filesystem.hpp>
#endif

//#include <yplatform/service/config.h>
#include <yamail/log/log_rotate.h>

YAMAIL_NS_BEGIN
YAMAIL_NS_LOG_BEGIN

#if BOOST_VERSION < 105400
#define YAMAIL_EXPORT BOOST_LOG_EXPORT
#else
#define YAMAIL_EXPORT BOOST_LOG_API
#endif

/*!
 * \brief An implementation of a text file logging sink backend
 *
 * The sink backend puts formatted log records to a text file.
 * The sink supports file rotation and advanced file control, such as
 * size and file count restriction.
 */
template< typename CharT >
class rotate_basic_text_file_backend :
#if BOOST_VERSION < 105400
    public boost::log::sinks::basic_formatting_sink_backend< CharT >,
#else
    public boost::log::sinks::basic_formatted_sink_backend< CharT >,
#endif
    public sink_support_rotate
{
    //! Base type
#if BOOST_VERSION < 105400
    typedef boost::log::sinks::basic_formatting_sink_backend< CharT > base_type;
#else
    typedef boost::log::sinks::basic_formatted_sink_backend< CharT > base_type;
#endif
    typedef boost::mutex mutex_t;
    typedef boost::unique_lock<mutex_t> lock_t;

public:
    typedef typename base_type::char_type char_type;
    typedef typename base_type::string_type string_type;
#if BOOST_VERSION < 105400
    typedef typename base_type::target_string_type target_string_type;
    typedef typename base_type::record_type record_type;
    typedef typename base_type::stream_type stream_type;
    typedef boost::log::aux::universal_path path_type;
#else
    typedef boost::filesystem::path path_type;
#endif

private:
    //! \cond

    struct implementation;
    implementation* m_pImpl;

    //! \endcond

public:
    /*!
     * Default constructor. The constructed sink backend uses default values of all the parameters.
     */
    YAMAIL_EXPORT rotate_basic_text_file_backend();

    /*!
     * Constructor. Creates a sink backend with the specified named parameters.
     * The following named parameters are supported:
     *
     * \li \c file_name - Specifies the file name pattern where logs are actually written to. The pattern may
     *                    contain directory and file name portions, but only the file name may contain
     *                    placeholders. The backend supports Boost.DateTime placeholders for injecting
     *                    current time and date into the file name. Also, an additional %N placeholder is
     *                    supported, it will be replaced with an integral increasing file counter. The placeholder
     *                    may also contain width specification in the printf-compatible form (e.g. %5N). The
     *                    printed file counter will always be zero-filled. If \c file_name is not specified,
     *                    pattern "%5N.log" will be used.
     * \li \c open_mode - File open mode. The mode should be presented in form of mask compatible to
     *                    <tt>std::ios_base::openmode</tt>. If not specified, <tt>trunc | out</tt> will be used.
     * \li \c auto_flush - Specifies a flag, whether or not to automatically flush the file after each
     *                     written log record. By default, is \c false.
     *
     * \note Read caution regarding file name pattern in the <tt>file::collector::scan_for_files</tt>
     *       documentation.
     */
#ifndef BOOST_LOG_DOXYGEN_PASS
    BOOST_LOG_PARAMETRIZED_CONSTRUCTORS_CALL(rotate_basic_text_file_backend, construct)
#else
    template< typename... ArgsT >
    explicit rotate_basic_text_file_backend(ArgsT... const& args);
#endif

    /*!
     * Destructor
     */
    YAMAIL_EXPORT ~rotate_basic_text_file_backend();

    /*!
     * The method sets the file open mode
     *
     * \param mode File open mode
     */
    YAMAIL_EXPORT void set_open_mode(std::ios_base::openmode mode);

    /*!
     * Sets the flag to automatically flush buffers of all attached streams after each log record
     */
    YAMAIL_EXPORT void auto_flush(bool f = true);

    /*!
     * The method sets file name path.
     *
     * \param pattern The name pattern for the file being written.
     */
    template< typename PathT >
    void set_file_name(PathT const& pattern)
    {
#if BOOST_VERSION < 105400
        set_file_name_internal(boost::log::aux::to_universal_path(pattern));
#else
        set_file_name_internal(path_type(pattern));
#endif
    }

    //! The method rotates the file
    void rotate();

    //! The method writes the message to the sink
#if BOOST_VERSION < 105400
    BOOST_LOG_EXPORT void do_consume(record_type const& record, target_string_type const& formatted_message);
#else
    YAMAIL_EXPORT void consume(boost::log::record_view const& rec, string_type const& formatted_message);
#endif

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! Constructor implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
#if BOOST_VERSION < 105400
            boost::log::aux::to_universal_path(args[boost::log::keywords::file_name | path_type()]),
#else
            path_type(args[boost::log::keywords::file_name | path_type()]),
#endif
            args[boost::log::keywords::open_mode | (std::ios_base::app)],
            args[boost::log::keywords::auto_flush | false]);
    }
    //! Constructor implementation
    YAMAIL_EXPORT void construct(
        path_type const& file_name,
        std::ios_base::openmode mode,
        bool auto_flush);

    //! The method sets file name
    YAMAIL_EXPORT void set_file_name_internal(path_type const& file_name);

    mutex_t mux_;
#endif // BOOST_LOG_DOXYGEN_PASS
};

#ifdef BOOST_LOG_USE_CHAR
typedef rotate_basic_text_file_backend< char > rotate_text_file_backend;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef rotate_basic_text_file_backend< wchar_t > rotate_wtext_file_backend;
#endif

YAMAIL_NS_LOG_END
YAMAIL_NS_END

#endif // _YAMAIL_LOG_ROTATE_TEXT_FILE_BACKEND_H_
