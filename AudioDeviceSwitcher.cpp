#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <comdef.h>

struct IPolicyConfig : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetMixFormat(PCWSTR, WAVEFORMATEX **) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR, INT, WAVEFORMATEX **) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(PCWSTR) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR, WAVEFORMATEX *, WAVEFORMATEX *) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR, INT, PINT64, PINT64) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR, PINT64) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR, void *) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR, void *) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(LPCWSTR wszDeviceId, ERole eRole) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR, INT) { return E_NOTIMPL; }
};

extern "C" __declspec(dllexport) int SetDefaultAudioDevice(const wchar_t* deviceName)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return 1;

    IMMDeviceEnumerator* pEnum = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (FAILED(hr)) return 2;

    IMMDeviceCollection* pDevices = nullptr;
    hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
    if (FAILED(hr)) { pEnum->Release(); return 3; }

    UINT count;
    pDevices->GetCount(&count);

    LPWSTR chosenId = nullptr;

    for (UINT i = 0; i < count; i++) {
        IMMDevice* pDev = nullptr;
        if (SUCCEEDED(pDevices->Item(i, &pDev))) {
            LPWSTR id = nullptr;
            if (SUCCEEDED(pDev->GetId(&id))) {
                IPropertyStore* pProps = nullptr;
                if (SUCCEEDED(pDev->OpenPropertyStore(STGM_READ, &pProps))) {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))) {
                        if (varName.vt == VT_LPWSTR && varName.pwszVal && wcsstr(varName.pwszVal, deviceName) != nullptr) {
                            chosenId = id;
                            pProps->Release();
                            pDev->Release();
                            break;
                        }
                    }
                    PropVariantClear(&varName);
                    pProps->Release();
                }
            }
            pDev->Release();
        }
    }

    if (!chosenId) { pDevices->Release(); pEnum->Release(); CoUninitialize(); return 4; }

    IPolicyConfig* pPolicyConfig = nullptr;
    CLSID clsid;
    CLSIDFromString(L"{870af99c-171d-4f9e-af0d-e63df40c2bc9}", &clsid);
    hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL, __uuidof(IPolicyConfig), (LPVOID*)&pPolicyConfig);
    if (FAILED(hr)) { pDevices->Release(); pEnum->Release(); CoUninitialize(); return 5; }

    pPolicyConfig->SetDefaultEndpoint(chosenId, eConsole);
    pPolicyConfig->SetDefaultEndpoint(chosenId, eMultimedia);
    pPolicyConfig->SetDefaultEndpoint(chosenId, eCommunications);

    pPolicyConfig->Release();
    pDevices->Release();
    pEnum->Release();
    CoUninitialize();
    return 0;
}
