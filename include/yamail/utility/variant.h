#ifndef _YAMAIL_UTILITY_VARIANT_H_
#define _YAMAIL_UTILITY_VARIANT_H_
#include <yamail/config.h>

#if YAMAIL_CPP < 11

# error This file requires compiler and library support for the \
ISO C++ 2011 standard. This support is currently experimental, and must be \
enabled with the -std=c++11 or -std=gnu++11 compiler options.

#else

#include <yamail/utility/namespace.h>

#include <cassert>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <utility>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

// See
// codereview.stackexchange.com/questions/28939/movable-and-copyable-variant

namespace detail
{
template <typename A, typename ...B>
struct max_align_type
{
  using type = typename ::std::conditional<
    (alignof(A) > alignof(typename max_align_type<B...>::type)),
    A,
    typename max_align_type<B...>::type
  >::type;
};

template <typename A, typename B>
struct max_align_type<A, B>
{
  using type = typename ::std::conditional<
    (alignof(A) > alignof(B)), A, B>::type;
};

template <typename A>
struct max_align_type<A>
{
  using type = A;
};

template <typename A, typename ...B>
struct max_size_type
{
  using type = typename ::std::conditional<
    (sizeof(A) > sizeof(typename max_size_type<B...>::type)),
    A,
    typename max_size_type<B...>::type
  >::type;
};

template <typename A, typename B>
struct max_size_type<A, B>
{
  using type = typename ::std::conditional<
    (sizeof(A) > sizeof(B)), A, B>::type;
};

template <typename A>
struct max_size_type<A>
{
  using type = A;
};

template <typename A, typename B, typename... C>
struct index_of :
  ::std::integral_constant<int,
    ::std::is_same<A, B>{} ?
    0 :
    (-1 == index_of<A, C...>{}) ? -1 : 1 + index_of<A, C...>{}
  >
{
};

template <typename A, typename B>
struct index_of<A, B> :
  ::std::integral_constant<int, ::std::is_same<A, B>{} - 1>
{
};

template <typename A, typename... B>
struct has_duplicates :
  ::std::integral_constant<bool,
    (-1 == index_of<A, B...>{} ? has_duplicates<B...>{} : true)
  >
{
};

template <typename A>
struct has_duplicates<A> :
  ::std::integral_constant<bool, false>
{
};

template <typename A, typename B, typename... C>
struct compatible_index_of :
  ::std::integral_constant<int,
    ::std::is_constructible<A, B>{} ?
      0 :
      (-1 == compatible_index_of<A, C...>{}) ?
        -1 :
        1 + compatible_index_of<A, C...>{}
  >
{
};

template <typename A, typename B>
struct compatible_index_of<A, B> :
  ::std::integral_constant<int, ::std::is_constructible<A, B>{} - 1>
{
};

template <typename A, typename B, typename... C>
struct compatible_type
{
  using type = typename ::std::conditional<
      ::std::is_constructible<A, B>{},
      B,
      typename compatible_type<A, C...
    >::type
  >::type;
};

template <typename A, typename B>
struct compatible_type<A, B>
{
  using type = typename ::std::conditional<
    ::std::is_constructible<A, B>{}, B, void>::type;
};

template <class S, class C, typename = void>
struct is_streamable : ::std::false_type { };

template <class S, class C>
struct is_streamable<S,
  C,
  decltype(void(sizeof(decltype(::std::declval<S&>()
    << ::std::declval<C const&>()))))
> : ::std::true_type
{
};

template < ::std::size_t I, typename A, typename ...B>
struct type_at : type_at<I - 1, B...>
{
};

template <typename A, typename ...B>
struct type_at<0, A, B...>
{
  using type = A;
};

template <bool B>
using bool_constant = ::std::integral_constant<bool, B>;

template <class A, class ...B>
struct all_of : bool_constant<A::value && all_of<B...>::value>
{
};

template <class A>
struct all_of<A> : bool_constant<A::value>
{
};

template <class A, class ...B>
struct any_of : bool_constant<A::value || any_of<B...>::value>
{
};

template <class A>
struct any_of<A> : bool_constant<A::value>
{
};

template <class A>
struct is_move_or_copy_constructible :
  bool_constant< ::std::is_copy_constructible<A>{} ||
    ::std::is_move_constructible<A>{}>
{
};

}

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif // __GNUC__

template <typename... T>
struct variant
{
  static_assert(!detail::any_of< ::std::is_reference<T>...>{},
    "reference types are unsupported");
  static_assert(!detail::any_of< ::std::is_void<T>...>{},
    "void type is unsupported");
  static_assert(detail::all_of<
    detail::is_move_or_copy_constructible<T>...>{},
    "unmovable and uncopyable types are unsupported");
  static_assert(!detail::has_duplicates<T...>{},
    "duplicate types are unsupported");

  using max_align_type = typename detail::max_align_type<T...>::type;

  using max_size_type = typename detail::max_size_type<T...>::type;

  static constexpr auto const max_align = alignof(max_align_type);

  variant() = default;

  ~variant()
  {
    if (*this)
    {
      deleter_(store_);
    }
    // else do nothing
  }

  variant(variant const& other) { *this = other; }

  variant(variant&& other) { *this = ::std::move(other); }

  variant& operator=(variant const& rhs)
  {
    if (!rhs)
    {
      if (*this)
      {
        store_type_ = -1;

        deleter_(store_);
      }
      // else do nothing
    }
    else if (rhs.copier_)
    {
      rhs.copier_(*this, rhs);
    }
    else
    {
      throw ::std::bad_typeid();
    }

    return *this;
  }

  variant& operator=(variant&& rhs)
  {
    if (!rhs)
    {
      if (*this)
      {
        store_type_ = -1;

        deleter_(store_);
      }
      // else do nothing
    }
    else if (rhs.mover_)
    {
      rhs.mover_(*this, ::std::move(rhs));
    }
    else
    {
      throw ::std::bad_typeid();
    }

    return *this;
  }

  template <
    typename U,
    typename = typename ::std::enable_if< detail::any_of< ::std::is_same<
      typename ::std::remove_reference<U>::type, T>...>{} &&
      !::std::is_same<typename ::std::decay<U>::type, variant>{}
    >::type
  >
  variant(U&& f)
  {
    *this = ::std::forward<U>(f);
  }

  template <typename S = ::std::ostream, typename U>
  typename ::std::enable_if< detail::any_of< ::std::is_same<
    typename ::std::remove_reference<U>::type, T>...>{} &&
    !::std::is_rvalue_reference<U&&>{} &&
    ::std::is_copy_assignable<typename ::std::remove_reference<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& f)
  {
    using user_type = typename ::std::remove_reference<U>::type;

    if (detail::index_of<user_type, T...>{} == store_type_)
    {
      *static_cast<user_type*>(static_cast<void*>(store_)) = f;
    }
    else
    {
      if (*this)
      {
        deleter_(store_);
      }
      // else do nothing

      new (store_) user_type(::std::forward<U>(f));

      deleter_ = destructor_stub<user_type>;

      copier_ = get_copier<user_type>();

      mover_ = get_mover<user_type>();

      streamer_ = get_streamer<S, user_type>();

      store_type_ = detail::index_of<user_type, T...>{};
    }

    return *this;
  }

  template <typename S = ::std::ostream, typename U>
  typename ::std::enable_if<
    detail::any_of< ::std::is_same<
      typename ::std::remove_reference<U>::type, T>...>{} &&
    ::std::is_rvalue_reference<U&&>{} &&
    ::std::is_move_assignable<typename ::std::remove_reference<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& f)
  {
    using user_type = typename ::std::remove_reference<U>::type;

    if (detail::index_of<user_type, T...>{} == store_type_)
    {
      *static_cast<user_type*>(static_cast<void*>(store_)) = ::std::move(f);
    }
    else
    {
      if (*this)
      {
        deleter_(store_);
      }
      // else do nothing

      new (store_) user_type(::std::forward<U>(f));

      deleter_ = destructor_stub<user_type>;

      copier_ = get_copier<user_type>();

      mover_ = get_mover<user_type>();

      streamer_ = get_streamer<S, user_type>();

      store_type_ = detail::index_of<user_type, T...>{};
    }

    return *this;
  }

  template <typename S = ::std::ostream, typename U>
  typename ::std::enable_if<
    detail::any_of< ::std::is_same<
      typename ::std::remove_reference<U>::type, T>...>{} &&
    !::std::is_copy_assignable<
      typename ::std::remove_reference<U>::type>{} &&
    !::std::is_move_assignable<
      typename ::std::remove_reference<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& f)
  {
    using user_type = typename ::std::remove_reference<U>::type;

    if (*this)
    {
      deleter_(store_);
    }
    // else do nothing

    new (store_) user_type(::std::forward<U>(f));

    deleter_ = destructor_stub<user_type>;

    copier_ = get_copier<user_type>();

    mover_ = get_mover<user_type>();

    streamer_ = get_streamer<S, user_type>();

    store_type_ = detail::index_of<user_type, T...>{};

    return *this;
  }

  explicit operator bool() const noexcept { return -1 != store_type_; }

  template <typename S = ::std::ostream, typename U>
  variant& assign(U&& f)
  {
    return operator=<S>(::std::forward<U>(f));
  }

  template <typename U>
  bool contains() const noexcept
  {
    return detail::index_of<U, T...>{} == store_type_;
  }

  bool empty() const noexcept { return !*this; }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  cget() const
  {
    if (detail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U const*>(static_cast<void const*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}) &&
    !(::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U const&
  >::type
  cget() const
  {
    if (detail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U const*>(static_cast<void const*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}),
    U&
  >::type
  get()
  {
    if (detail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U*>(static_cast<void*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}),
    U const&
  >::type
  get() const
  {
    if (detail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U const*>(static_cast<void const*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 == detail::index_of<U, T...>{}) &&
    (-1 != detail::compatible_index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  get() const
  {
    static_assert(::std::is_same<
      typename detail::type_at<
        detail::compatible_index_of<U, T...>{}, T...>::type,
      typename detail::compatible_type<U, T...>::type>{},
      "internal error");
    if (detail::compatible_index_of<U, T...>{} == store_type_)
    {
      return U(*static_cast<
        typename detail::compatible_type<U, T...>::type const*>(
          static_cast<void const*>(store_)));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  static constexpr int type_index() noexcept
  {
    return detail::index_of<U, T...>{};
  }

  int type_index() const noexcept { return store_type_; }

private:
  using copier_type = void (*)(variant&, variant const&);
  using mover_type = void (*)(variant&, variant&&);
  using streamer_type = void (*)(void*, variant const&);

  template <typename charT, typename traits>
  friend ::std::basic_ostream<charT, traits>& operator<<(
    ::std::basic_ostream<charT, traits>& os, variant const& v)
  {
    v.streamer_(&os, v);

    return os;
  }

  template <class U>
  constexpr typename ::std::enable_if<
    ::std::is_copy_constructible<U>{}, copier_type
  >::type
  get_copier() const
  {
    return copier_stub<U>;
  }

  template <class U>
  constexpr typename ::std::enable_if<
    !::std::is_copy_constructible<U>{}, copier_type
  >::type
  get_copier() const
  {
    return nullptr;
  }

  template <class U>
  constexpr typename ::std::enable_if<
    ::std::is_move_constructible<U>{}, mover_type
  >::type
  get_mover() const
  {
    return mover_stub<U>;
  }

  template <class U>
  constexpr typename ::std::enable_if<
    !::std::is_move_constructible<U>{}, mover_type
  >::type
  get_mover() const
  {
    return nullptr;
  }

  template <class S, class U>
  constexpr typename ::std::enable_if<
    detail::is_streamable<S, U>{},
    streamer_type
  >::type
  get_streamer() const
  {
    return streamer_stub<S, U>;
  }

  template <class S, class U>
  constexpr typename ::std::enable_if<
    !detail::is_streamable<S, U>{},
    streamer_type
  >::type
  get_streamer() const
  {
    return nullptr;
  }

  template <typename U>
  static void destructor_stub(void* const p)
  {
    static_cast<U*>(p)->~U();
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_copy_constructible<U>{} &&
    ::std::is_copy_assignable<U>{}
  >::type
  copier_stub(variant& dst, variant const& src)
  {
    if (src.store_type_ == dst.store_type_)
    {
      *static_cast<U*>(static_cast<void*>(dst.store_)) =
        *static_cast<U const*>(static_cast<void const*>(src.store_));
    }
    else
    {
      if (dst)
      {
        dst.deleter_(dst.store_);
      }
      // else do nothing

      new (dst.store_) U(*static_cast<U const*>(
        static_cast<void const*>(src.store_)));

      dst.deleter_ = src.deleter_;

      dst.copier_ = src.copier_;

      dst.mover_ = src.mover_;

      dst.streamer_ = src.streamer_;

      dst.store_type_ = src.store_type_;
    }
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_copy_constructible<U>{} &&
    !::std::is_copy_assignable<U>{}
  >::type
  copier_stub(variant& dst, variant const& src)
  {
    if (dst)
    {
      dst.deleter_(dst.store_);
    }
    // else do nothing

    new (dst.store_) U(*static_cast<U const*>(
      static_cast<void const*>(src.store_)));

    dst.deleter_ = src.deleter_;

    dst.copier_ = src.copier_;

    dst.mover_ = src.mover_;

    dst.streamer_ = src.streamer_;

    dst.store_type_ = src.store_type_;
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_move_constructible<U>{} &&
    ::std::is_move_assignable<U>{}
  >::type
  mover_stub(variant& dst, variant&& src)
  {
    if (src.store_type_ == dst.store_type_)
    {
      *static_cast<U*>(static_cast<void*>(dst.store_)) =
        ::std::move(*static_cast<U*>(static_cast<void*>(src.store_)));
    }
    else
    {
      if (dst)
      {
        dst.deleter_(dst.store_);
      }
      // else do nothing

      new (dst.store_) U(::std::move(*static_cast<U*>(
        static_cast<void*>(src.store_))));

      dst.deleter_ = src.deleter_;

      dst.copier_ = src.copier_;

      dst.mover_ = src.mover_;

      dst.streamer_ = src.streamer_;

      dst.store_type_ = src.store_type_;
    }
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_move_constructible<U>{} &&
    !::std::is_move_assignable<U>{}
  >::type
  mover_stub(variant& dst, variant&& src)
  {
    if (dst)
    {
      dst.deleter_(dst.store_);
    }
    // else do nothing

    new (dst.store_) U(::std::move(*static_cast<U*>(
      static_cast<void*>(src.store_))));

    dst.deleter_ = src.deleter_;

    dst.copier_ = src.copier_;

    dst.mover_ = src.mover_;

    dst.streamer_ = src.streamer_;

    dst.store_type_ = src.store_type_;
  }

  template <class S, typename U>
  static typename ::std::enable_if<
    detail::is_streamable<S, U>{}
  >::type
  streamer_stub(void* const os, variant const& v)
  {
    *static_cast<S*>(os) << v.cget<U>();
  }

  using deleter_type = void (*)(void*);
  deleter_type deleter_;

  copier_type copier_;
  mover_type mover_;

  streamer_type streamer_;

  int store_type_{-1};

  alignas(max_align_type) char store_[sizeof(max_size_type)];
};

template <typename U, typename... T>
auto cget (variant<T...> const& v) -> decltype (v.template cget<U> ())
{
	return v.template cget<U> ();
}

template <typename U, typename... T>
auto get (variant<T...>& v) -> decltype (v.template get<U> ())
{
	return v.template get<U> ();
}

template <typename U, typename... T>
constexpr int type_index(variant<T...> const&) noexcept
{
	return variant<T...>::template type_index<U> ();
}

template <typename... T>
int type_index(variant<T...> const& v) noexcept 
{ 
	return v.type_index (); 
}


#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif // __GNUC__

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION

#endif // C++11

#endif // _YAMAIL_UTILITY_VARIANT_H_
