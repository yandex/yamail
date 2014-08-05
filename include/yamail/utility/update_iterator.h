#ifndef _YAMAIL_UTILITY_UPDATE_ITERATOR_H_
#define _YAMAIL_UTILITY_UPDATE_ITERATOR_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <yamail/compat/basics.h>

#include <iterator>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

template <class Container>
class update_iterator 
  : public std::iterator<std::output_iterator_tag,void,void,void,void>
{
protected:
  Container* container;

public:
  typedef Container container_type;

  explicit update_iterator (Container& x)
    : container (YAMAIL_FQNS_COMPAT::addressof (x)) {}

  update_iterator<Container>&
  operator= (const typename Container::value_type& value)
  { 
  	std::pair<typename Container::iterator, bool> ret = 
  	    container->insert(value);

  	if (! ret.second) ret.first->second = value;
  	return *this; 
  }

#if 0 // YAMAIL_CPP >= 11
  update_iterator<Container>& operator= (typename Container::value_type&& value)
    { iter=container->insert(iter,std::move(value)); ++iter; return *this; }
#endif

  update_iterator<Container>& operator* ()
    { return *this; }
  update_iterator<Container>& operator++ ()
    { return *this; }
  update_iterator<Container> operator++ (int)
    { return *this; }
}; 

template <class Container>
update_iterator<Container> 
updater (Container& x)
{
	return update_iterator<Container> (x);
}

#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::utility
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION


#endif // _YAMAIL_UTILITY_UPDATE_ITERATOR_H_
