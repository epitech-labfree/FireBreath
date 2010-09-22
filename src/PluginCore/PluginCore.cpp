/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Oct 19, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass, Inc and the Firebreath development team
\**********************************************************/

#include "PluginWindow.h"
#include "JSAPI.h"
#include "variant_list.h"
#include "FactoryDefinitions.h"
#include "BrowserHostWrapper.h"
#include "DOM/Window.h"

#include "PluginCore.h"

using namespace FB;

/***************************\
 Static initialization stuff
\***************************/

volatile int PluginCore::ActivePluginCount = 0;

std::string PluginCore::OS;
std::string PluginCore::Browser;
void PluginCore::setPlatform(const std::string& os, const std::string& browser)
{
    PluginCore::OS = os;
    PluginCore::Browser = browser;
}

/***************************\
     Regular Class Stuff
\***************************/

PluginCore::PluginCore() : m_Window(NULL), m_paramsSet(false)
{
    // This class is only created on the main UI thread,
    // so there is no need for mutexes here
    if (++PluginCore::ActivePluginCount == 1) {
        // Only on the first initialization
        GlobalPluginInitialize();
    }

    initDefaultParams();
}

PluginCore::PluginCore(const std::set<std::string>& params)
    : m_Window(NULL), m_supportedParamSet(params), m_paramsSet(false)
{
    // This class is only created on the main UI thread,
    // so there is no need for mutexes here
    if (++PluginCore::ActivePluginCount == 1) {
        // Only on the first initialization
        GlobalPluginInitialize();
    }
    initDefaultParams();
}

void PluginCore::initDefaultParams()
{
    m_supportedParamSet.insert("onload");
    m_supportedParamSet.insert("onerror");
    m_supportedParamSet.insert("onlog");
}

PluginCore::~PluginCore()
{
    // This class is only destroyed on the main UI thread,
    // so there is no need for mutexes here
    if (--PluginCore::ActivePluginCount == 0) {
        // Only on the destruction of the final plugin instance
        GlobalPluginDeinitialize();
    }
}

StringSet* PluginCore::getSupportedParams()
{
    return &m_supportedParamSet;
}

void PluginCore::setParams(const FB::VariantMap& inParams)
{
    m_params.insert(inParams.begin(), inParams.end());
    for (FB::VariantMap::iterator it = m_params.begin(); it != m_params.end(); ++it)
    {
        std::string key(it->first);
        try {
            std::string value(it->second.convert_cast<std::string>());
            if (key.substr(0, 2) == "on") {
                FB::JSObject tmp;
                tmp = m_host->getDOMWindow()
                    ->getProperty<FB::JSObject>(value);

                m_params[key] = tmp;
            }
        } catch (std::exception &ex) {
            printf(ex.what());
        }
    }
}

void PluginCore::SetHost(FB::BrowserHost host)
{
    m_host = host;
}

JSAPIPtr PluginCore::getRootJSAPI()
{
    if (!m_api) {
        m_api = createJSAPI();
    }

    return m_api;
}

PluginWindow* PluginCore::GetWindow() const
{
    return m_Window;
}

void PluginCore::SetWindow(PluginWindow *wind)
{
    if (m_Window && m_Window != wind) {
        ClearWindow();
    }
    m_Window = wind;
    wind->AttachObserver(this);
}

void PluginCore::ClearWindow()
{
    if (m_Window) {
        m_Window->DetachObserver(this);
        m_Window = NULL;
    }
}

// If you override this, you probably want to call it again, since this is what calls back into the page
// to indicate that we're done.
void PluginCore::setReady()
{
    try {
        FB::VariantMap::iterator fnd = m_params.find("onload");
        if (fnd != m_params.end()) {
            FB::JSObject method = fnd->second.convert_cast<FB::JSObject>();
            method->InvokeAsync("", FB::variant_list_of(getRootJSAPI()));
        }
    } catch(...) {
        // Usually this would be if it isn't a JSObject or the object can't be called
    }
}