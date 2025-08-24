#define UNICODE
#define _UNICODE
#include "RtdServer.h"
#include <iostream>
#include <olectl.h>

class CComFactory : public IClassFactory {
  private:
    ULONG m_RefCount = 0;

  public:
    HRESULT STDMETHODCALLTYPE QueryInterface(const IID &riid, void **ppv) {
        if (IID_IUnknown == riid) {
            *ppv = static_cast<IUnknown *>(this);
        } else if (IID_IClassFactory == riid) {
            *ppv = static_cast<IClassFactory *>(this);
        } else {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        static_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() {
        m_RefCount++;
        return m_RefCount;
    }

    ULONG STDMETHODCALLTYPE Release() {
        m_RefCount--;
        if (0 == m_RefCount) {
            delete this;
            return 0;
        }
        return m_RefCount;
    }

    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *pUnkOuter, const IID &riid, void **ppvObject) {
        if (NULL != pUnkOuter) {
            return CLASS_E_NOAGGREGATION;
        }
        HRESULT hr = E_OUTOFMEMORY;
        RtdServer *pObj = new RtdServer();
        if (NULL == pObj) {
            return hr;
        }
        hr = pObj->QueryInterface(riid, ppvObject);
        if (S_OK != hr) {
            delete pObj;
        }
        return hr;
    }
    HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) {
        return NOERROR;
    }
};

/*************
 * DLL注册表信息
 * 这里定义了RTDServer的ProgId、CLSID、TypeId等信息
 * 这些信息用于COM注册和查找
 *************/
const WCHAR *regTable[][3] = {
    {RtdServer_ProgId, 0, RtdServer_ProgId},
    {RtdServer_ProgId L"\\CLSID", 0, RtdServer_CLSID},

    {L"CLSID\\" RtdServer_CLSID, 0, RtdServer_ProgId},
    {L"CLSID\\" RtdServer_CLSID L"\\InprocServer32", 0, (const WCHAR *)-1},
    {L"CLSID\\" RtdServer_CLSID L"\\ProgId", 0, RtdServer_ProgId},
};

STDAPI DllUnregisterServer() {
    HRESULT hr = S_OK;
    int n = sizeof(regTable) / sizeof(*regTable);
    for (int i = n - 1; i >= 0; i--) {
        const WCHAR *key = regTable[i][0];
        long err = RegDeleteKeyW(HKEY_CLASSES_ROOT, key);
        if (err != ERROR_SUCCESS) {
            hr = SELFREG_E_CLASS;
        }
    }
    return hr;
}

STDAPI DllRegisterServer() {
    if (RtdServer_DllName[0] == L'\0') {
        return -1;
    }

    int n = sizeof(regTable) / sizeof(*regTable);
    for (int i = 0; i < n; i++) {
        const WCHAR *key = regTable[i][0];
        const WCHAR *valueName = regTable[i][1];
        const WCHAR *value = regTable[i][2];

        if (value == (const WCHAR *)-1) {
            value = RtdServer_DllName;
        }

        HKEY hkey;
        long err = RegCreateKey(HKEY_CLASSES_ROOT, key, &hkey);
        if (err == ERROR_SUCCESS) {
            err = RegSetValueEx(hkey, valueName, 0, REG_SZ, (const BYTE *)value, (lstrlen(value) + 1) * sizeof(WCHAR));
            RegCloseKey(hkey);
        }

        if (err != ERROR_SUCCESS) {
            DllUnregisterServer();
            return SELFREG_E_CLASS;
        }
    }
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv) {
    if (CLSID_RtdServer != rclsid) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
    CComFactory *pFactory = new CComFactory();
    if (NULL == pFactory) {
        return E_OUTOFMEMORY;
    }
    HRESULT result = pFactory->QueryInterface(riid, ppv);
    return result;
}

STDAPI DllCanUnloadNow() {
    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        GetModuleFileName(hModule, RtdServer_DllName, sizeof(RtdServer_DllName) / sizeof(WCHAR));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}