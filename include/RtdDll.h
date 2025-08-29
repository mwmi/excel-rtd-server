#pragma once
#define UNICODE
#include "RtdServer.h"
#include <iostream>
#include <olectl.h>

#ifdef USER_REG
#define REG_HKEY HKEY_CURRENT_USER
#define REG_PROGID_KEY L"SOFTWARE\\Classes\\" RtdServer_ProgId
#define REG_CLSID_KEY  L"SOFTWARE\\Classes\\CLSID\\" RtdServer_CLSID
#else
#define REG_HKEY HKEY_CLASSES_ROOT
#define REG_PROGID_KEY RtdServer_ProgId
#define REG_CLSID_KEY  L"CLSID\\" RtdServer_CLSID
#endif

class CComFactory : public IClassFactory {
private:
    ULONG m_RefCount = 0;

public:
    HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, const IID& riid, void** ppvObject);
    HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);
};

STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv);
STDAPI DllCanUnloadNow();