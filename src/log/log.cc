#include <boost/version.hpp>

#if BOOST_VERSION < 104600
# include "log-144.h"
#elif BOOST_VERSION >= 104600 && BOOST_VERSION < 105400
# include "log-146.h"
#elif BOOST_VERSION >= 105400
# include "log-154.h"
#endif


