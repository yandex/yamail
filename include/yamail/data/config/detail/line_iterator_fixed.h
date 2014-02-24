#ifndef _YAMAIL_DATA_CONFIG_DETAIL_line_iterator_fixed_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_line_iterator_fixed_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

// Fixed version of line_pos_iterator get_line_start
template <class Iterator>
inline Iterator 
get_line_start_FIXED (Iterator lower_bound, Iterator current)
{
   bool e = false;
   Iterator latest = lower_bound;
   if (lower_bound != current)
   {
     for (Iterator i = lower_bound; ; ++i)
     {
       switch (*i) {
         case '\r': case '\n': latest = i; e = true; break;
         default: if (e) { latest = i; e = false; }
       }

       if (i == current) break;
     }
   }
   return latest;
}

template <class Iterator>
inline std::size_t 
get_column_FIXED (Iterator lower_bound,
                               Iterator current,
                               std::size_t tabs = 4)
{
     std::size_t column = 1;
     Iterator first = get_line_start_FIXED(lower_bound, current);

     for (Iterator i = first; i != current; ++i) {
       switch (*i) {
         case '\t':
           column += tabs - (column - 1) % tabs;
           break;
         default:
           ++column;
       }
     }

     return column;
}

template <typename Iterator>
inline boost::iterator_range<Iterator>
get_current_line_FIXED (
    Iterator lower_bound, Iterator current, Iterator upper_bound)
{
   // get line start
   Iterator line_start = get_line_start_FIXED (lower_bound, current);

   // get line end
   Iterator line_end = line_start;
   while (line_end != upper_bound && *line_end != '\r' && *line_end != '\n')
     ++line_end;

   return boost::make_iterator_range (line_start, line_end);

}

} // namespace detail
YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_DETAIL_line_iterator_fixed_H_
