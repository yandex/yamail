#ifndef _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_OPENER_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_OPENER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/compat/shared_ptr.h>

#include <yamail/data/config/ast.h>
#include <yamail/data/config/paths_type.h>
#include <yamail/data/config/detail/boost_fs_handler.h>

#include <yamail/utility/in.h>

#include <list>
#include <sstream>
#include <memory>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <vector>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

using utility::rw_in;

template <
    typename Parser
  , typename FsHandler = boost_fs_handler<>
  , typename IncludeDirs = std::vector<typename FsHandler::path_type>
  , typename AllocT = std::allocator<ast> 
>
class include_opener
{
public:
  typedef AllocT allocator_type;
  typedef Parser parser_type;

  typedef FsHandler fs_handler_type;
  typedef typename fs_handler_type::path_type path_type;

  typedef IncludeDirs include_dirs_type;

  typedef compat::shared_ptr<ast> ast_ptr;

protected:
  typedef std::list<path_type> stack_type;

  struct fs_guard {
    typename fs_handler_type::file_type& fs;
    fs_guard (typename fs_handler_type::file_type& fs) : fs (fs) {}
    ~fs_guard () { fs.close (); }
  };

  struct stack_guard {
    stack_type& stack_;
    template <class T> stack_guard (stack_type& s, T&& t) : stack_ (s) 
    { s.push_back (std::forward<T> (t)); }
    ~stack_guard () { stack_.pop_back (); }
  };


public:
  include_opener (rw_in<parser_type> const& parser, fs_handler_type& fs, 
      rw_in<include_dirs_type> const& dirs = include_dirs_type (),
      allocator_type const& alloc = allocator_type ())
    : parser_ (parser)
    , fs_handler_ (fs)
    , dirs_ (dirs)
    , alloc_ (alloc)
  {
  }

  inline allocator_type& get_allocator ()
  {
    return alloc_;
  }

  inline path_type normalize (path_type const& path) const
  {
    return fs_handler_.normalize_path_impl (path);
  }

  inline bool is_absolute (path_type const& path) const
  {
    return fs_handler_.is_absolute (path);
  }

  // NB: rw argument!
  ast_ptr operator() (path_type const& path)
  {
    return get_normalized (normalize (path));
  }

  ast_ptr get_normalized (path_type const& path)
  {
    if (std::find (stack_.begin (), stack_.end (), path) != stack_.end ())
    {
      std::ostringstream os;
      os << "recursive include detected: ";
      std::for_each (stack_.begin (), stack_.end (),
        [&os] (path_type const& x) { os << x << " "; }
      );

      // TODO: signal by error_handler instead?
      throw std::runtime_error (os.str ());
    }

    compat::shared_ptr<ast> nodes (compat::allocate_shared<ast> (alloc_));

    {
      stack_guard sg (stack_, path);

      typename fs_handler_type::file_type file;
      
      file = fs_handler_.open (path);
      fs_guard guard (file);


      // parse iterators
      // bool ret = 
        parser_ (*nodes, path, file.begin (), file.end (), fs_handler_);
#if 0
      ret = detail::parse (*nodes, path, file.begin (), file.end (), 
          error_wrapper_, include_dirs_, fs_handler_, *this);
#endif
    }

    return nodes;
  }

private:
#if 1
  parser_type&        parser_;
#else
  error_wrapper_type& error_wrapper_;
#endif
  fs_handler_type&    fs_handler_;
  include_dirs_type&  dirs_;
  stack_type          stack_;
  allocator_type      alloc_;
};


} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_OPENER_H_
