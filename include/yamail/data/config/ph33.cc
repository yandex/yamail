#include <boost/phoenix/function/adapt_callable.hpp>
#include <boost/phoenix.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <iostream>
#include <cassert>

namespace px = boost::phoenix;


namespace demo
{
using boost::remove_reference;
    struct plus
    {
        template <typename Sig>
        struct result;

        template <typename This, typename A0, typename A1>
        struct result<This(A0, A1)>
            : remove_reference<A0>
        {};

        template <typename This, typename A0, typename A1, typename A2>
        struct result<This(A0, A1, A2)>
            : remove_reference<A0>
        {};

        template <typename A0, typename A1>
        A0 operator()(A0 const & a0, A1 const & a1) 
        {
            return a0 + a1;
        }

        template <typename A0, typename A1, typename A2>
        A0 operator()(A0 const & a0, A1 const & a1, A2 const & a2) const
        {
            return a0 + a1 + a2;
        }
    };
}

BOOST_PHOENIX_ADAPT_CALLABLE(plus, demo::plus, 2)

BOOST_PHOENIX_ADAPT_CALLABLE(plus, demo::plus, 3)

int main()
{
    using boost::phoenix::arg_names::arg1;
    using boost::phoenix::arg_names::arg2;

    int a = 123;
    int b = 256;

    assert(plus(arg1, arg2)(a, b) == a+b);
    assert(plus(arg1, arg2, 3)(a, b) == a+b+3);
}
