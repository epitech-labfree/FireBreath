/**********************************************************\
Original Author: Richard Bateman (taxilian)

Created:    Oct 19, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#include <cstdio>
#include "config.h"
#include "NpapiPluginModule.h"
#include "NpapiPlugin.h"
#include "FactoryBase.h"
#include "NpapiBrowserHost.h"
#include <boost/shared_ptr.hpp>
#include "AsyncFunctionCall.h"
#include "SafeQueue.h"

#if FB_WIN
#  include "Win/NpapiBrowserHostAsyncWin.h"
#elif FB_MACOSX
#  include "Mac/OneShotManager.h"
#endif

using namespace FB::Npapi;

namespace
{
    bool needAsyncCallsWorkaround(NPP npp, NPNetscapeFuncs* funcs)
    {
        // work-around detection here
#ifdef _WINDOWS_
#if 0
        const char* const cstrUserAgent = funcs->uagent(npp);
        if(!cstrUserAgent) 
            return false;
        
        const std::string userAgent(cstrUserAgent);        
        const bool result = userAgent.find("Opera") != std::string::npos;
        return result;
#endif
#endif
        return (funcs->version < NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL);
    }

    bool asyncCallsWorkaround(NPP npp, NPNetscapeFuncs* funcs = 0)
    {
        static const bool useWorkaround = (funcs) ? needAsyncCallsWorkaround(npp, funcs) : false;
        //static const bool useWorkaround = false;
        return useWorkaround;
    }

    NpapiBrowserHostPtr createBrowserHost(NpapiPluginModule* module, NPP npp)
    {
        try {
            NpapiBrowserHostPtr host;
            NPNetscapeFuncs& npnFuncs = module->NPNFuncs;

            if(asyncCallsWorkaround(npp, &npnFuncs)) {
                npnFuncs.pluginthreadasynccall = NULL;
    #ifdef _WINDOWS_
                NpapiBrowserHostPtr host(new NpapiBrowserHostAsyncWin(module, npp));
                return host;
    #else
                // no work-around for this platform
                NpapiBrowserHostPtr host(new NpapiBrowserHost(module, npp));
                return host;
    #endif
            } else {
                NpapiBrowserHostPtr host(new NpapiBrowserHost(module, npp));
                return host;
            }
        } catch (...) {
            // This function must not return an exception
            return NpapiBrowserHostPtr();
        }
    }

    struct NpapiPDataHolder
    {
        NpapiBrowserHostPtr host;
        boost::shared_ptr<NpapiPlugin> plugin;
        FB::SafeQueue< FB::AsyncFunctionCallPtr > asyncFunctionQueue;

        NpapiPDataHolder(NpapiBrowserHostPtr host, boost::shared_ptr<NpapiPlugin> plugin)
          : host(host), plugin(plugin) {}
        ~NpapiPDataHolder() {}
    };

    

    NpapiPDataHolder* getHolder(NPP instance)
    {   
        return static_cast<NpapiPDataHolder*>(instance->pdata);
    }

    NpapiBrowserHostPtr getHost(NPP instance)
    {
        return static_cast<NpapiPDataHolder*>(instance->pdata)->host;
    }

    bool validInstance(NPP instance)
    {
        return instance != NULL && instance->pdata != NULL;
    }
    
    void asyncCallbackFunction(NPP npp, uint32_t timerID)
    {
        boost::shared_ptr<FB::AsyncFunctionCall> evt;
        NpapiPDataHolder *holder = getHolder(npp);
        while (holder->asyncFunctionQueue.try_pop(evt)) {
            evt->func(evt->userData);
        }
    }    
}

NpapiPluginPtr FB::Npapi::getPlugin(NPP instance)
{
    return static_cast<NpapiPDataHolder*>(instance->pdata)->plugin;
}


// This is used on mac snow leopard safari
void NpapiPluginModule::scheduleAsyncCallback(NPP npp, void (*func)(void *), void *userData)
{
    getHolder(npp)->asyncFunctionQueue.push(FB::AsyncFunctionCallPtr(new FB::AsyncFunctionCall(func, userData)));
    //getHost(npp)->ScheduleTimer(0, false, &asyncCallbackFunction);
#ifdef FB_MACOSX
    OneShotManager::getInstance().push(npp, &asyncCallbackFunction);
#endif
}

NpapiPluginModule *NpapiPluginModule::Default = NULL;

// These are the static NPP_ functions; NPP_New and NPP_Destroy create and destroy the
// plugin, the rest are wrappers that dereference NPP->pdata to get at the plugin object
// and proxy the call to there.
NPError NpapiPluginModule::NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc,
                                   char* argn[], char* argv[], NPSavedData* saved)
{
    if (instance == NULL) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    try 
    {
        NPNetscapeFuncs& npnFuncs = NpapiPluginModule::Default->NPNFuncs;
        
        NpapiBrowserHostPtr host(createBrowserHost(NpapiPluginModule::Default, instance));
        host->setBrowserFuncs(&(npnFuncs));

        // TODO: We should probably change this and pass the MIMEType into _getNpapiPlugin instead
        // of into init later so that we can optionally return a different plugin type depending
        // on the specific mimetype
        NpapiPluginPtr plugin(getFactoryInstance()->createNpapiPlugin(host, pluginType));
        if (!plugin) {
            return NPERR_OUT_OF_MEMORY_ERROR;
        }

        NpapiPDataHolder* holder = new NpapiPDataHolder(host, plugin);
        instance->pdata = static_cast<void*>(holder);

        plugin->init(pluginType, argc, argn, argv);
    } 
    catch (const PluginCreateError &e) 
    {
        printf("%s\n", e.what());
        return NPERR_INCOMPATIBLE_VERSION_ERROR;
    }
    catch (const std::bad_alloc& e) 
    {
        printf("%s\n", e.what());
        return NPERR_OUT_OF_MEMORY_ERROR;
    }
    catch (const std::exception& e) 
    {
        printf("%s\n", e.what());
        return NPERR_GENERIC_ERROR;
    }

    return NPERR_NO_ERROR;
}

NPError NpapiPluginModule::NPP_Destroy(NPP instance, NPSavedData** save)
{
	if (NpapiBrowserHostPtr host = getHost(instance)) {
		host->shutdown();
	}
#ifdef FB_MACOSX
    OneShotManager::getInstance().clear(instance);
#endif
    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        plugin->shutdown();
    }

    delete getHolder(instance);
    instance->pdata = NULL;

    return NPERR_NO_ERROR;
}

NPError NpapiPluginModule::NPP_SetWindow(NPP instance, NPWindow* window)
{
    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if(asyncCallsWorkaround(instance)) {
        NpapiBrowserHostPtr host = getHost(instance);
        if(host) {
#ifdef _WINDOWS_
            boost::shared_ptr<NpapiBrowserHostAsyncWin> hostWin = boost::dynamic_pointer_cast<NpapiBrowserHostAsyncWin>(host);
            if(hostWin)
                hostWin->setWindow(window);
#endif
        }
    }

#if FB_GUI_DISABLED != 1
    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->SetWindow(window);
    }
#endif

    return NPERR_NO_ERROR;
}

NPError NpapiPluginModule::NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream,
                                       NPBool seekable, uint16_t* stype)
{
    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->NewStream(type, stream, seekable, stype);
    } else {    
        return NPERR_GENERIC_ERROR;
    }
}

NPError NpapiPluginModule::NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{
    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->DestroyStream(stream, reason);
    } else {
        return NPERR_GENERIC_ERROR;
    }
}

int32_t NpapiPluginModule::NPP_WriteReady(NPP instance, NPStream* stream)
{
    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->WriteReady(stream);
    } else {    
        return NPERR_GENERIC_ERROR;
    }
}

int32_t NpapiPluginModule::NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len,
                                 void* buffer)
{
    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->Write(stream, offset, len, buffer);
    } else {
        return NPERR_GENERIC_ERROR;
    }
}

void NpapiPluginModule::NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
    if (!validInstance(instance)) {
        return;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        plugin->StreamAsFile(stream, fname);
    }
}

void NpapiPluginModule::NPP_Print(NPP instance, NPPrint* platformPrint)
{
    if (!validInstance(instance)) {
        return;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        plugin->Print(platformPrint);
    }
}

int16_t NpapiPluginModule::NPP_HandleEvent(NPP instance, void* event)
{
    if (!validInstance(instance)) {
        return 0;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->HandleEvent(event);
    } else {
        return 0;
    }
}

void NpapiPluginModule::NPP_URLNotify(NPP instance, const char* url, NPReason reason,
                                    void* notifyData)
{
    if (!validInstance(instance)) {
        return;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        plugin->URLNotify(url, reason, notifyData);
    }
}

NPError NpapiPluginModule::NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
    /* Some values are not dependent on having a valid plugin instance */
    switch(variable) {
        case NPPVpluginNameString:
            *((const char **)value) = FBSTRING_PluginName;
            return NPERR_NO_ERROR;
            break;
        case NPPVpluginDescriptionString:
            *((const char **)value) = FBSTRING_FileDescription;
            return NPERR_NO_ERROR;
            break;
        default:
            break;
    }

    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->GetValue(variable, value);
    }

    return NPERR_NO_ERROR;
}

NPError NpapiPluginModule::NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
    if (!validInstance(instance)) {
        return NPERR_INVALID_INSTANCE_ERROR;
    }

    if (NpapiPluginPtr plugin = getPlugin(instance)) {
        return plugin->SetValue(variable, value);
    } else {
        return NPERR_GENERIC_ERROR;
    }
}
