class holder;
class error {};

template <class T = holder, class E = error> class expected;

template <class E> class expected<void, E>;

template <class E> class expected<holder, E>;

template <typename T, typename E>
class expected
{
public:
  typedef T value_type;
  typedef E error_type;

  template <class U> struct rebind {
  	typedef expected<U, error_type> type;
  };

  constexpr T const& value() const& { return val; }

#if 0
  T& value() &;
  constexpr T&& value() &&;
  constexpr E const& error() const&;
  E& error() &;
  constexpr E&& error() &&;
#endif

private:
  bool has_value; // exposition only
  union
  {
    value_type val; // exposition only
    error_type err; // exposition only
  };
};

int main ()
{
	expected<int> ex;
}
