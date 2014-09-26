#include <type_traits>

#if __cplusplus >= 201400
// Note: std::integer_sequence is C++14,
// but it's easy to use your own version (even stripped down)
// for the following purpose:
template< bool... Bs >
using bool_sequence = std::integer_sequence< bool, Bs... >;
#else
// Alternatively, not using C++14:
template< bool... > struct bool_sequence {};
#endif

template< bool... Bs >
using bool_and = std::is_same< bool_sequence< Bs... >,
                               bool_sequence< ( Bs || true )... > >;

template< bool... Bs >
using bool_or = std::integral_constant< bool, !bool_and< !Bs... >::value >;

template< typename R, bool... Bs > // note: R first, no default :(
using enable_if_any = std::enable_if< bool_or< Bs... >::value, R >;

template< typename R, bool... Bs > // note: R first, no default :(
using enable_if_all = std::enable_if< bool_and< Bs... >::value, R >;

template< typename T, typename... Ts >
using are_same = bool_and< std::is_same< T, Ts >::value... >;

template <typename ...A>
struct S 
{
	template <typename ...B>
	typename enable_if_all<
	    void
	  , std::is_constructible<A,B>::value...
	>::type
	foo (B... b) 
	{}
};

int main ()
{
	S<int, float> s;
	s.foo (5, 6);
}
