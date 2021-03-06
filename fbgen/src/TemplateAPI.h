/**********************************************************\

  Auto-generated @{PLUGIN_ident}API.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "@{PLUGIN_ident}.h"

#ifndef H_@{PLUGIN_ident}API
#define H_@{PLUGIN_ident}API

class @{PLUGIN_ident}API : public FB::JSAPIAuto
{
public:
    @{PLUGIN_ident}API(@{PLUGIN_ident}Ptr plugin, FB::BrowserHostPtr host);
    virtual ~@{PLUGIN_ident}API();

    @{PLUGIN_ident}Ptr getPlugin();

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

    // Method echo
    FB::variant echo(const FB::variant& msg);
    
    // Method test-event
    void testEvent(const FB::variant& s);

private:
    @{PLUGIN_ident}WeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_testString;
};

#endif // H_@{PLUGIN_ident}API

