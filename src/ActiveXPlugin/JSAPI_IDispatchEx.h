/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Oct 30, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#pragma once
#ifndef H_JSAPI_IDISPATCHEX
#define H_JSAPI_IDISPATCHEX

#include "ActiveXBrowserHost.h"
#include "APITypes.h"
#include "JSAPI.h"
#include "TypeIDMap.h"
#include "COM_config.h"
#include "utf8_tools.h"

#include "axmain.h"
#include "IDispatchAPI.h"
#include "dispex.h"
#include <map>
#include "logging.h"
#include <mshtmdid.h>

class JSAPI_IDispatchExBase
{
public:
    void setAPI(FB::JSAPIPtr api, ActiveXBrowserHostPtr host)
    {
        m_api = api;
        m_host = host;
    }

	FB::JSAPIPtr getAPI()
	{
		return m_api;
	}

protected:
    FB::JSAPIPtr m_api;
    ActiveXBrowserHostPtr m_host;
};

template <class T, class IDISP, const IID* piid>
class JSAPI_IDispatchEx :
    public JSAPI_IDispatchExBase,
    public IDISP,
    public IConnectionPointContainer,
    public IConnectionPoint
{
    typedef CComEnum<IEnumConnectionPoints, &__uuidof(IEnumConnectionPoints), IConnectionPoint*,
        _CopyInterface<IConnectionPoint> > CComEnumConnectionPoints;
    typedef std::map<DWORD, IDispatchAPIPtr> ConnectionPointMap;

public:
    JSAPI_IDispatchEx(void) : m_readyState(READYSTATE_LOADING) { };
    virtual ~JSAPI_IDispatchEx(void) { };
    void setReadyState(READYSTATE newState)
    {
        m_readyState = newState;
        if (m_propNotify.p != NULL)
            m_propNotify->OnChanged(DISPID_READYSTATE);
    }

protected:
    ConnectionPointMap m_connPtMap;

    READYSTATE m_readyState;
    CComQIPtr<IPropertyNotifySink, &IID_IPropertyNotifySink> m_propNotify;
    
    bool m_valid;
    std::vector<std::wstring> m_memberList;

    virtual bool callSetEventListener(const std::vector<FB::variant> &args, bool add);

public:
    /* IConnectionPointContainer members */
    HRESULT STDMETHODCALLTYPE EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    HRESULT STDMETHODCALLTYPE FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP);

    /* IConnectionPoint members */
    HRESULT STDMETHODCALLTYPE GetConnectionInterface(IID *pIID);
    HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(IConnectionPointContainer **ppCPC);
    HRESULT STDMETHODCALLTYPE Advise(IUnknown *pUnkSink, DWORD *pdwCookie);
    HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwCookie);
    HRESULT STDMETHODCALLTYPE EnumConnections(IEnumConnections **ppEnum);

    /* IDispatch members */
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid,
        DISPID *rgDispId);
    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, 
        DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

    /* IDispatchEx members */
    HRESULT STDMETHODCALLTYPE GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid);
    HRESULT STDMETHODCALLTYPE InvokeEx(DISPID id, LCID lcid, WORD wFlags,
        DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);
    HRESULT STDMETHODCALLTYPE DeleteMemberByName(BSTR bstrName, DWORD grfdex);
    HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(DISPID id);
    HRESULT STDMETHODCALLTYPE GetMemberProperties(DISPID id, DWORD grfdexFetch,
        DWORD *pgrfdex);
    HRESULT STDMETHODCALLTYPE GetMemberName(DISPID id, BSTR *pbstrName);
    HRESULT STDMETHODCALLTYPE GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid);
    HRESULT STDMETHODCALLTYPE GetNameSpaceParent(IUnknown **ppunk);
};

/* IConnectionPointContainer methods */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    if (ppEnum == NULL)
        return E_POINTER;
    *ppEnum = NULL;
    CComEnumConnectionPoints* pEnum = NULL;

    pEnum = new CComObject<CComEnumConnectionPoints>;
    if (pEnum == NULL)
        return E_OUTOFMEMORY;

    IConnectionPoint *connectionPoint[1] = { NULL };
    static_cast<T*>(this)->QueryInterface(IID_IConnectionPoint, (void **)connectionPoint);

    HRESULT hRes = pEnum->Init(connectionPoint, &connectionPoint[1],
        reinterpret_cast<IConnectionPointContainer*>(this), AtlFlagCopy);

    if (FAILED(hRes))
    {
        delete pEnum;
        return hRes;
    }
    hRes = pEnum->QueryInterface(__uuidof(IEnumConnectionPoints), (void**)ppEnum);
    if (FAILED(hRes))
        delete pEnum;
    return hRes;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP)
{
    if (ppCP == NULL)
        return E_POINTER;
    *ppCP = NULL;
    HRESULT hRes = CONNECT_E_NOCONNECTION;

    if (InlineIsEqualGUID(*piid, riid)) {
        hRes = static_cast<T*>(this)->QueryInterface(__uuidof(IConnectionPoint), (void**)ppCP);
    }

    return hRes;
}

/* IConnectionPoint methods */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetConnectionInterface(IID *pIID)
{
    *pIID = *piid;
    return S_OK;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
    if (ppCPC == NULL)
        return E_POINTER;

    return static_cast<T*>(this)->QueryInterface(__uuidof(IConnectionPointContainer), (void**)ppCPC);
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::Advise(IUnknown *pUnkSink, DWORD *pdwCookie)
{
    if (!m_api) return CONNECT_E_CANNOTCONNECT;

    IDispatch *idisp(NULL);
    if (SUCCEEDED(pUnkSink->QueryInterface(IID_IDispatch, (void**)&idisp))) {
        IDispatchAPIPtr obj(new IDispatchAPI(idisp, m_host));
        m_connPtMap[(DWORD)obj.get()] = obj;
        *pdwCookie = (DWORD)obj.get();
        m_api->registerEventInterface(FB::ptr_cast<FB::JSObject>(obj));
        return S_OK;
    } else {
        return CONNECT_E_CANNOTCONNECT;
    }
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::Unadvise(DWORD dwCookie)
{
    if (!m_api)
        return S_OK;    // If m_api has been detached already, the plugin is shut down so this is a non-issue
    ConnectionPointMap::iterator fnd = m_connPtMap.find(dwCookie);
    if (fnd == m_connPtMap.end()) {
        return E_UNEXPECTED;
    } else {
        m_api->registerEventInterface(FB::ptr_cast<FB::JSObject>(fnd->second));
        m_connPtMap.erase(fnd);
        return S_OK;
    }
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::EnumConnections(IEnumConnections **ppEnum)
{
    return E_NOTIMPL;
}

/* IDispatch methods */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, 
                                                  UINT cNames, LCID lcid, 
                                                  DISPID *rgDispId)
{
    HRESULT rv = S_OK;
    for (UINT i = 0; i < cNames; i++) {
        CComBSTR bstr(rgszNames[i]);
        rv = GetDispID(bstr, 0, &rgDispId[i]);
        if (!SUCCEEDED(rv)) {
            return rv;
        }
    }
    return rv;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
                                           WORD wFlags, DISPPARAMS *pDispParams, 
                                           VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                                           UINT *puArgErr)
{
    return InvokeEx(dispIdMember, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, NULL);
}


/* IDispatchEx members */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
    if (!m_api) return DISP_E_UNKNOWNNAME;

    std::wstring wsName(bstrName);

    if ((wsName == L"attachEvent") || (wsName == L"detachEvent")) {
        *pid = AxIdMap.getIdForValue(wsName);
    } else if (m_api->HasProperty(wsName) || m_api->HasMethod(wsName) || m_api->HasEvent(wsName)) {
        *pid = AxIdMap.getIdForValue(wsName);
    } else {
        *pid = -1;
    }
    return S_OK;
}

// helper method for Invoke
template <class T, class IDISP, const IID* piid>
bool JSAPI_IDispatchEx<T,IDISP,piid>::callSetEventListener(const std::vector<FB::variant> &args, bool add)
{
    if (args.size() < 2 || args.size() > 3
         || args[0].get_type() != typeid(std::string)
         || args[1].get_type() != typeid(FB::JSObjectPtr)) {
        throw FB::invalid_arguments();
    }

    std::string evtName = args[0].convert_cast<std::string>();
    FB::JSObjectPtr method(args[1].convert_cast<FB::JSObjectPtr>());
    if (add) {
        m_api->registerEventMethod(evtName, method);
    } else {
        m_api->unregisterEventMethod(evtName, method);
    }

    return true;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::InvokeEx(DISPID id, LCID lcid, WORD wFlags,
                                             DISPPARAMS *pdp, VARIANT *pvarRes, 
                                             EXCEPINFO *pei, 
                                             IServiceProvider *pspCaller)
{
    if (!m_api || !AxIdMap.idExists(id)) {
        return DISP_E_MEMBERNOTFOUND;
    }

    std::wstring wsName;

    try 
    {
        wsName = AxIdMap.getValueForId<std::wstring>(id);

        if (wFlags & DISPATCH_PROPERTYGET) {
            if(!pvarRes)
                return E_INVALIDARG;

            switch(id) {
                case DISPID_READYSTATE:
                    CComVariant(this->m_readyState).Detach(pvarRes);
                    return S_OK;
                case DISPID_ENABLED:
                    CComVariant(true).Detach(pvarRes);
                    return S_OK;
                case DISPID_SECURITYCTX:
                    return E_NOTIMPL;
            }
        }

        if (wFlags & DISPATCH_PROPERTYGET && m_api->HasProperty(wsName)) {

            if(!pvarRes)
                return E_INVALIDARG;

            FB::variant rVal = m_api->GetProperty(wsName);

            m_host->getComVariant(pvarRes, rVal);

        } else if ((wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF) && m_api->HasProperty(wsName)) {

            FB::variant newVal = m_host->getVariant(&pdp->rgvarg[0]);

            m_api->SetProperty(wsName, newVal);
        } else if ((wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF) && m_api->HasEvent(wsName)) {
            
            FB::variant newVal = m_host->getVariant(&pdp->rgvarg[0]);
            if (newVal.empty()) {
                m_api->setDefaultEventMethod(wsName, FB::JSObjectPtr());
            } else {
                FB::JSObjectPtr method(newVal.cast<FB::JSObjectPtr>());
                m_api->setDefaultEventMethod(wsName, method);
            }

        } else if (wFlags & DISPATCH_METHOD && ((wsName == L"attachEvent") || (wsName == L"detachEvent")) ) {
        
            std::vector<FB::variant> params;
            for (int i = pdp->cArgs - 1; i >= 0; i--) {
                params.push_back(m_host->getVariant(&pdp->rgvarg[i]));
            }

            if (wsName == L"attachEvent") {
                this->callSetEventListener(params, true);
            } else if (wsName == L"detachEvent") {
                this->callSetEventListener(params, false);
            }

        } else if (wFlags & DISPATCH_METHOD && m_api->HasMethod(wsName) ) {

            std::vector<FB::variant> params;
            for (int i = pdp->cArgs - 1; i >= 0; i--) {
                params.push_back(m_host->getVariant(&pdp->rgvarg[i]));
            }
            FB::variant rVal;
            rVal = m_api->Invoke(wsName, params);
            
            if(pvarRes)
                m_host->getComVariant(pvarRes, rVal);

        } else {
            throw FB::invalid_member("Invalid method or property name");
        }
    } catch (const FB::invalid_member&) {
        FBLOG_INFO("JSAPI_IDispatchEx", "No such member: \"" << FB::wstring_to_utf8(wsName) << "\"");
        return DISP_E_MEMBERNOTFOUND;
    } catch (const FB::script_error& se) {
        FBLOG_INFO("JSAPI_IDispatchEx", "Script error for \"" << FB::wstring_to_utf8(wsName) << "\": " << se.what());
        if (pei != NULL) {
            pei->bstrSource = CComBSTR(ACTIVEX_PROGID);
            pei->bstrDescription = CComBSTR(se.what());
            pei->bstrHelpFile = NULL;
            pei->pfnDeferredFillIn = NULL;
            pei->scode = E_NOTIMPL;
        }
        return DISP_E_EXCEPTION;
    } catch (...) {
        return E_NOTIMPL;
    }

    return S_OK;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::DeleteMemberByDispID(DISPID id)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetMemberProperties(DISPID id, DWORD grfdexFetch,
                                                        DWORD *pgrfdex)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetMemberName(DISPID id, BSTR *pbstrName)
{
    try {
        CComBSTR outStr(AxIdMap.getValueForId<std::wstring>(id).c_str());

        *pbstrName = outStr.Detach();
        return S_OK;
    } catch (...) {
        return DISP_E_UNKNOWNNAME;
    }
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
    if (!m_api)
        return S_FALSE;

    if (m_memberList.size() != m_api->getMemberCount()) {
        m_memberList.clear();
        m_api->getMemberNames(m_memberList);
    }
    if (id == DISPID_STARTENUM) {
        *pid = AxIdMap.getIdForValue(m_memberList[0]);
        return S_OK;
    } 
    std::wstring wStr = AxIdMap.getValueForId<std::wstring>(id);

    std::vector<std::wstring>::iterator it;
    for (it = m_memberList.begin(); it != m_memberList.end(); it++) {
        if (wStr == *it) {
            it++;
            break;
        }
    }
    if (it != m_memberList.end()) {
        *pid = AxIdMap.getIdForValue(*it);
    } else {
        return S_FALSE;
    }
    return S_OK;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetNameSpaceParent(IUnknown **ppunk)
{
    return E_NOTIMPL;
}

#endif // H_JSAPI_IDISPATCHEX
