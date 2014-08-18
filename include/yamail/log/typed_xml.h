#ifndef _YAMAIL_LOG_TYPED_XML_H_
#define _YAMAIL_LOG_TYPED_XML_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/logic/tribool.hpp>

#include <boost/foreach.hpp>

#include <map>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION
namespace typed {

class definitions
{
	static std::map<std::string, attribute_def>& 
	map () _noexcept
	{
		static std::map<std::string, attribute_def> m;
		return m;
  }

  static const boost::property_tree::ptree&
  empty_ptree () _noexcept
  {
  	static const boost::property_tree::ptree t;
  	return t;
  }

public:
  void reload_xml (std::string const& path)
  {
  	using boost::property_tree::ptree;
    using boost::algorithm::iequals;
    ptree pt;

    std::map<std::string, attribute_def> m;

    read_xml (path, pt);

    BOOST_FOREACH (ptree::value_type const& f, pt.get_child ("log.fields"))
    {
      using boost::tribool;
      tribool has_name = false;
      tribool has_type = false;
      tribool has_rule = false;

      if (! iequals (f.first, "field"))
      {
        BOOST_THROW_EXCEPTION (std::runtime_error ("bad field in XML"));
      }

      attribute_def def = { "", TYPE_STRING, RULE_OPTIONAL, "" };

      def.descr = f.second.data ();

      BOOST_FOREACH (ptree::value_type const& a, 
            f.second.get_child ("<xmlattr>", empty_ptree ()))
      {
        if (iequals (a.first, "name"))
        {
          def.name = a.first;
          has_name = !has_name || boost::indeterminate;
        }
        else if (iequals (a.first, "type"))
        {
               if (iequals (a.second.data (), "number")) 
                  def.type = TYPE_NUMBER;
          else if (iequals (a.second.data (), "real")) 
          	      def.type = TYPE_REAL;
          else if (iequals (a.second.data (), "string")) 
          	      def.type = TYPE_STRING;
          else if (iequals (a.second.data (), "ip")) 
          	      def.type = TYPE_IP;
          else if (iequals (a.second.data (), "endpoint")) 
          	      def.type = TYPE_IP_ENDPOINT;
          else if (iequals (a.second.data (), "time")) 
          	      def.type = TYPE_TIME;
          else {
            BOOST_THROW_EXCEPTION (std::runtime_error ("bad field type"));
          }
          has_type = !has_type || boost::indeterminate;
        }
        else if (iequals (a.first, "rule"))
        {
               if (iequals (a.second.data (), "mandatory")) 
               	    def.rule = RULE_MANDATORY;
          else if (iequals (a.second.data (), "optional")) 
          	        def.rule = RULE_OPTIONAL;
          else if (iequals (a.second.data (), "ignored")) 
          	        def.rule = RULE_IGNORED;
          else if (iequals (a.second.data (), "strict-ignored")) 
          	        def.rule = RULE_STRICT_IGNORED;
          else if (iequals (a.second.data (), "generated")) 
          	        def.rule = RULE_GENERATED;
          else if (iequals (a.second.data (), "auto")) 
          	        def.rule = RULE_AUTO;
          else {
            BOOST_THROW_EXCEPTION (std::runtime_error ("bad field rule"));
          }
          has_rule = !has_rule || boost::indeterminate;
        }
      }

      if (!has_name)
      {
        BOOST_THROW_EXCEPTION (
          std::runtime_error ("absent name attribute for field")
        );
      }

      if (indeterminate (has_name))
      {
        BOOST_THROW_EXCEPTION (
          std::runtime_error ("several name attributes for field")
        );
      }

      if (indeterminate (has_rule))
      {        
      	BOOST_THROW_EXCEPTION (
          std::runtime_error ("several rule attributes for field")
        );
      }

      if (indeterminate (has_type))
      {
        BOOST_THROW_EXCEPTION (
          std::runtime_error ("several type attributes for field")
        );
      }

      m.insert (std::make_pair (def.name, def));
    }

    std::swap (map (), m);
  }
};

} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_XML_H_
