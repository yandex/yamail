#ifndef _YAMAIL_DATA_SELECT_MAP_H_
#define _YAMAIL_DATA_SELECT_MAP_H_
#include <yamail/config.h>
#include <yamail/data/namespace.h>

#include <yamail/traits/is_complete.h>
#include <yamail/compat/unordered.h> // unordered_map

// MPL
#include <boost/mpl/if.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>

// hashes
#include <functional>
#include <boost/functional/hash.hpp>

#include <boost/parameter.hpp>
#include <boost/type_traits/is_same.hpp>

YAMAIL_NS_BEGIN
YAMAIL_NS_DATA_BEGIN

namespace keywords {

struct hashed {};
struct ordered {};

BOOST_PARAMETER_TEMPLATE_KEYWORD(key_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(value_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(hash_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(equal_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(cmp_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(index_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(alloc_type)

} // namespace keywords

namespace detail {

using YAMAIL_FQNS_TRAITS::is_complete;
using namespace keywords;

namespace mpl = boost::mpl;

using mpl::not_;
using mpl::or_;
using mpl::and_;
using mpl::if_;
using mpl::_;

using boost::is_same;

struct default_ {};

template <typename T> struct is_default : boost::is_same<T, default_> {};
template <typename T> struct is_not_default : not_<is_default<T> > {};

template <typename T> struct is_index_type: boost::false_type {};
template <> struct is_index_type<hashed>: boost::true_type {};
template <> struct is_index_type<ordered>: boost::true_type {};

template <typename T> struct is_hash: boost::false_type {};
template <typename T> struct is_hash<std::hash<T> >: boost::true_type {};
template <typename T> struct is_hash<boost::hash<T> >: boost::true_type {};

template <typename T> struct is_alloc: boost::false_type {};
template <typename T> struct is_alloc<std::allocator<T> >: boost::true_type {};

template <typename T> struct is_less: boost::false_type {};
template <typename T> struct is_less<std::less<T> >: boost::true_type {};

template <typename T> struct is_equal_to: boost::false_type {};
template <typename T> struct is_equal_to<std::equal_to<T> >:boost::true_type {};

namespace parameter = boost::parameter;

typedef parameter::parameters<
    parameter::required<keywords::tag::key_type>

  , parameter::required<keywords::tag::value_type>

  , parameter::optional<
        parameter::deduced<keywords::tag::index_type>
      , is_index_type<_>
    >

  , parameter::optional<
        parameter::deduced<keywords::tag::hash_type>
      , not_<
          or_<is_index_type<_>, is_less<_>, is_equal_to<_>, is_alloc<_> >
        >
    >

  , parameter::optional<
        parameter::deduced<keywords::tag::equal_type>
      , not_<
          or_<is_index_type<_>, is_less<_>, is_hash<_>, is_alloc<_> >
        >
    >

  , parameter::optional<
        parameter::deduced<keywords::tag::cmp_type>
      , not_<
          or_<is_index_type<_>, is_hash<_>, is_equal_to<_>, is_alloc<_> >
        >
    >

  , parameter::optional<
        parameter::deduced<keywords::tag::alloc_type>
      , not_<
          or_<is_index_type<_>, is_hash<_>, is_equal_to<_>, is_less<_> >
        >
    >
> select_map_spec;



template <class A0, class A1,
          class A2 = parameter::void_, class A3 = parameter::void_,
          class A4 = parameter::void_, class A5 = parameter::void_
> class select_map
{
  typedef typename select_map_spec::bind<A0,A1,A2,A3,A4,A5>::type args;

  typedef typename parameter::value_type<args, keywords::tag::key_type>::type 
    key_type;

  typedef typename parameter::value_type<args, keywords::tag::value_type>::type
    value_type;

  typedef typename parameter::value_type<
    args, keywords::tag::hash_type, default_>::type hash_type_;

  typedef typename parameter::value_type<
    args, keywords::tag::equal_type, default_>::type equal_type_;

  typedef typename parameter::value_type<
    args, keywords::tag::cmp_type, default_>::type cmp_type_;

  typedef typename parameter::value_type<args, keywords::tag::index_type,
          default_>::type index_type_;

  // set default allocator
  typedef typename parameter::value_type<args, keywords::tag::alloc_type,
          std::allocator<std::pair<const key_type, value_type> > >::type
            alloc_type;

  // if index_type not defined then ...
  //   if defined hash or equal_to functors then index_type := hashed
  //   else if defined less functor the index_type := ordered
  
  typedef typename if_<is_not_default<index_type_>
        , index_type_
        , typename if_<
              or_<is_not_default<hash_type_>, 
                       is_not_default<equal_type_> >
            , keywords::hashed
            , typename if_<is_not_default<cmp_type_>, 
                                keywords::ordered, index_type_>::type
          >::type
  >::type index_type_1;

#if 0 // boost::hash it not working as expected
  template <typename X> using set_hash =
    typename if_<is_complete<std::hash<X> >,
      std::hash<X>, typename if_<is_complete<boost::hash<X> >,
      boost::hash<X>, default_>::type
    >::type;
#else
  template <typename X> using set_hash =
    typename if_<is_complete<std::hash<X> >,
             std::hash<X>, default_>::type;
#endif

  // if index_type_1 is not ordered then set hash functor to std or boost hash
  typedef typename if_<
      and_<is_default<hash_type_>, 
        not_<is_same<index_type_1, keywords::ordered> > >,
      set_hash<key_type>, hash_type_>::type hash_type;

  template <typename X> using set_equal =
    typename if_<is_complete<std::equal_to<X> >,
             std::equal_to<X>, default_>::type;

  // if index_type_1 is not ordered then set equal functor to std::equal_to
  typedef typename if_<
      and_<is_default<equal_type_>, 
        not_<is_same<index_type_1, keywords::ordered> > >,
      set_equal<key_type>, equal_type_>::type equal_type;

  template <typename X> using set_cmp =
    typename if_<is_complete<std::less<X> >,
             std::less<X>, default_>::type;

  // if index_type_1 is not ordered then set equal functor to std::equal_to
  typedef typename if_<
      and_<is_default<cmp_type_>, 
        not_<is_same<index_type_1, keywords::hashed> > >,
      set_cmp<key_type>, cmp_type_>::type cmp_type;

  // if index_type_1 is not defined then set it depending on hash and cmp
  // functors.
  typedef typename if_<
      is_default<index_type_1>
    , typename if_<
          and_<is_not_default<hash_type>
                  , is_not_default<equal_type> >
        , keywords::hashed
        , typename if_<
              is_not_default<cmp_type>
            , keywords::ordered
            , index_type_1
          >::type
      >::type
    , index_type_1
  >::type index_type;

  static_assert (!is_default<index_type>::value, "cannot deduce index type");

  static_assert (
      (is_same<index_type, hashed>::value && is_not_default<hash_type>::value)
   || (is_same<index_type, ordered>::value && is_not_default<cmp_type>::value)
   , "cannot deduce comparator");

  template <class I, class K, class V, class H, class P, class C,
            class A = alloc_type>
  struct impl;

  template <class K, class V, class H, class P, class C, class A>
  struct impl<keywords::hashed,K,V,H,P,C,A>
  { 
    typedef compat::unordered_map<K, V, H, P, A> type;
  };

  template <class K, class V, class H, class P, class C, class A>
  struct impl<keywords::ordered,K,V,H,P,C,A>
  { 
    typedef std::map<K, V, C, A> type;
  };

public:
  typedef typename impl<index_type, key_type, value_type, hash_type,
          equal_type, cmp_type>::type type;
};

} // namespace detail

template <class A0, class A1,
          class A2 = boost::parameter::void_, 
          class A3 = boost::parameter::void_,
          class A4 = boost::parameter::void_,
          class A5 = boost::parameter::void_>
using select_map = typename detail::select_map<A0,A1,A2,A3,A4,A5>::type;

YAMAIL_NS_DATA_END
YAMAIL_NS_END

#endif // _YAMAIL_DATA_SELECT_MAP_H_
