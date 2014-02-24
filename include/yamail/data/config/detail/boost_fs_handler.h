#ifndef _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/compat/shared_ptr.h>
#include <yamail/data/config/detail/fs_handler.h>

#include <iterator>
#include <type_traits>

#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/functional/hash.hpp>

// extend std::hash for boost::filesystem::path
// this enables to use boost::filesystem::path with std::unordered_maps
namespace std {
template <> struct hash<boost::filesystem::path>
{
  inline size_t operator() (const boost::filesystem::path& x) const
  {
    return boost::hash<boost::filesystem::path> () (x);
  }
};
} // namespace std

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

struct boost_fs_error : std::ios_base::failure
{
  boost_fs_error (std::string const& x) 
    : std::ios_base::failure::failure (x)
  {
  }
};

template <typename CharT = char>
class boost_fs_handler;

template <typename CharT = char>
class boost_fs_file
  : public boost::iterator_range<CharT const*>
{
  typedef CharT char_type;
  typedef boost::iterator_range<char_type const*> base_t;
  compat::shared_ptr<io::mapped_file_source> file_;
  
public:
  typedef typename boost_fs_handler<char_type>::error_type error_type;

protected:
  friend class boost_fs_handler<char_type>;

  boost_fs_file (std::string const& path) try
    : file_ (compat::make_shared<io::mapped_file_source> (path))
  {
    this->base_t::operator= ( 
      boost::make_iterator_range (file_->data (), 
        file_->data () + file_->size ())
    );
  }
  catch (BOOST_IOSTREAMS_FAILURE const& error)
  {
    throw error_type (error.what ());
  }

public:
  boost_fs_file () {}

  void close () 
  {
    file_.reset ();
  }
};


template <typename CharT>
class boost_fs_handler
{
  typedef boost::iterator_range<CharT const*> base_t;

  compat::shared_ptr<io::mapped_file_source> file_;

public:
  typedef boost_fs_error error_type;

  typedef return_iterator_range category;
  typedef CharT char_type;
  typedef boost_fs_file<char_type> file_type;

  typedef fs::path path_type;

  file_type open (path_type const& path)
  {
    // just in case
    // fs::path normalized_path = normalize_path_impl (path);
    return file_type (path.template string<std::string> ());
  }

  inline void close (file_type& file)
  {
    file.close ();
  }

  inline static bool is_absolute (path_type const& path)
  {
    return path.has_root_directory ();
  }

#if 1
  static path_type normalize_path_impl (
      path_type const& path,
      const path_type& base=fs::current_path())
  {
    fs::path abs_path = absolute (path, base);
    fs::path::iterator it = abs_path.begin ();
    fs::path result = *it++;

    // Get canonical version of the existing part
    for (; exists (result) && it != abs_path.end (); ++it)
      result /= *it;

    result = canonical (result.parent_path ());

    // For the rest remove ".." and "." in a path with no symlinks
    for (--it; it != abs_path.end (); ++it)
    {
      // Move back on "..".
      if (*it == "..")
        result = result.parent_path ();

      // Ignore ".".
      else if (*it != ".")
        result /= *it;
    }

    return result;
  }
#else
  template <typename C, typename T, typename A>
  static inline 
  std::basic_string<C,T,A>
  normalize_path (std::basic_string<C,T,A> const& path)
  {
    return 
      normalize_path_impl (path).template string<std::basic_string<C,T,A> > ();
  }

  template <typename C, std::size_t N>
  static inline 
  std::basic_string<typename std::remove_const<C>::type>
  normalize_path (C (&path)[N])
  {
    typedef typename std::remove_const<C>::type char_type;

    return normalize_path_impl (path).
      template string<std::basic_string<char_type> > ();
  }

  template <typename Iterator>
  static inline
  std::basic_string<
    typename std::decay<
      typename std::iterator_traits<Iterator>::value_type
    >::type
  >
  normalize_path (Iterator path)
  {
    typedef typename std::decay<
      typename std::iterator_traits<Iterator>::value_type
    >::type char_type;

    return normalize_path_impl (path).
      template string<std::basic_string<char_type> > ();
  }

  template <typename Dest, typename Src>
  static inline Dest normalize_path_raw (Src const& path)
  {
    return normalize_path_impl (path). template string<Dest> ();
  }

protected:

  template <typename Source>
  static fs::path normalize_path_impl (Source const& path)
  {
    fs::path abs_path = absolute (fs::path (path));
    fs::path::iterator it = abs_path.begin ();
    fs::path result = *it++;

    // Get canonical version of the existing part
    for (; exists (result) && it != abs_path.end (); ++it)
      result /= *it;

    result = canonical (result.parent_path ());

    // For the rest remove ".." and "." in a path with no symlinks
    for (--it; it != abs_path.end (); ++it)
    {
      // Move back on "..".
      if (*it == "..")
        result = result.parent_path ();

      // Ignore ".".
      else if (*it != ".")
        result /= *it;
    }

    return result;
  }
#endif
};

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_
