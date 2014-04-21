#ifndef _YAMAIL_UTILITY_APPLY_H_
#define _YAMAIL_UTILITY_APPLY_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

YAMAIL_NS_BEGIN
YAMAIL_NS_UTILITY_BEGIN

// See http://stackoverflow.com/a/12650100/633160

#if _cplusplus >= 201300L // C++1y

template<typename F, typename Tuple, std::size_t ... I>
auto apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template<typename F, typename Tuple>
auto apply(F&& f, Tuple&& t) {
    using Indices = 
        std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>;
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t), Indices());
}

#else 

template<size_t N>
struct Apply {
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T && t, A &&... a)
        -> decltype(Apply<N-1>::apply(
            ::std::forward<F>(f), ::std::forward<T>(t),
            ::std::get<N-1>(::std::forward<T>(t)), ::std::forward<A>(a)...
        ))
    {
        return Apply<N-1>::apply(::std::forward<F>(f), ::std::forward<T>(t),
            ::std::get<N-1>(::std::forward<T>(t)), ::std::forward<A>(a)...
        );
    }
};

template<>
struct Apply<0> {
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T &&, A &&... a)
        -> decltype(::std::forward<F>(f)(::std::forward<A>(a)...))
    {
        return ::std::forward<F>(f)(::std::forward<A>(a)...);
    }
};

template<typename F, typename T, typename... A>
inline auto apply(F && f, T && t, A &&... a)
    -> decltype(Apply< ::std::tuple_size<
        typename ::std::decay<T>::type
    >::value>::apply(::std::forward<F>(f), ::std::forward<T>(t),
     ::std::forward<A> (a)...))
{
    return Apply< ::std::tuple_size<
        typename ::std::decay<T>::type
    >::value>::apply(::std::forward<F>(f), ::std::forward<T>(t),
      ::std::forward<A> (a)...);
}

#endif // C++14

YAMAIL_NS_UTILITY_END
YAMAIL_NS_END

#endif // _YAMAIL_UTILITY_APPLY_H_
