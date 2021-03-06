/**********************************************************\ 
Original Author: Georg Fritzsche

Created:    Dec 16, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Georg Fritzsche, Firebreath development team
\**********************************************************/

#ifndef H_FAKE_JS_ARRAY
#define H_FAKE_JS_ARRAY

#include "JSObject.h"

class FakeJsArray : public FB::JSObject
{
public:
    FakeJsArray(const FB::VariantList& values)
      : FB::JSObject(FB::BrowserHostPtr()), m_values(values)
    {
    }
    
    FB::JSAPIPtr getJSAPI() { return FB::JSAPIPtr(); }

    bool HasMethod(const std::string&) { return false; }
    void SetProperty(int, const FB::variant&) {}
    void SetProperty(const std::string&, const FB::variant) {}
    FB::variant Invoke(const std::string&, const FB::VariantList&) { return FB::variant(); }

    // Methods for enumeration
    virtual void getMemberNames(std::vector<std::string> &nameVector) { }
    virtual size_t getMemberCount() { return 0; }

    bool HasProperty(const std::string& s)    
    { 
        return (s == "length"); 
    }
    
    bool HasProperty(int index)        
    { 
        return ((unsigned)index < m_values.size()); 
    }
    
    FB::variant GetProperty(const std::string& s) 
    { 
        if(s == "length")
            return (int)m_values.size();
        throw FB::script_error(std::string("no such property '")+s+"'");
    }
    
    FB::variant GetProperty(int index)     
    { 
        if((unsigned)index >= m_values.size()) {
            std::stringstream ss;
            ss << "index out of range - got " << index << ", size is " << m_values.size();
            throw FB::script_error(ss.str());
        }
        return m_values[index]; 
    }
    
    void SetProperty(const std::string& name, const FB::variant& value) {}

private:
    FB::VariantList m_values;
};

#endif
