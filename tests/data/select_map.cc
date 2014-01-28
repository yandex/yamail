#include <yamail/data/select_map.h>
#include <yamail/compat/unordered.h>
#include <type_traits>

int main ()
{
  namespace data = YAMAIL_FQNS_DATA;
  namespace compat = YAMAIL_FQNS_COMPAT;


  using std::is_same;
  using data::select_map;
  namespace kwd = data::keywords;
  using compat::unordered_map;

  // select_map<int, char> m;

  static_assert (
      (is_same<select_map<int, char>, 
       unordered_map<int, char, std::hash<int>> >::value), 
      "bad select_map<int, char>");

  enum Enum { A, B, C };

  static_assert (
      (is_same<select_map<Enum, char, boost::hash<Enum> >, 
       unordered_map<Enum, char, boost::hash<Enum>> >::value), 
      "bad select_map<Enum, char, boost::hash<Enum> >");

  static_assert (
      (is_same<select_map<Enum, char>, 
       std::map<Enum, char> >::value), 
      "bad select_map<Enum, char>");

  static_assert (
      (is_same<select_map<Enum, char, kwd::ordered>, 
       std::map<Enum, char> >::value), 
      "bad select_map<Enum, char, ordered>");


  struct Struct 
  {
    void* v;

    bool operator< (Struct const&) const
    {
      return 0;
    }
  } s;

  select_map<Struct, char> n2;
  n2[s] = 'd';

  static_assert (
      (is_same<data::select_map<Struct, char>, 
       std::map<Struct, char> >::value), 
      "bad select_map<Struct, char>");
}
