/*
 * List Windows Audio Devices
 * Shows all available audio capture devices
 */

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <iostream>

#pragma comment(lib, "ole32.lib")

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IMMDevice = __uuidof(IMMDevice);
const IID IID_IAudioClient = __uuidof(IAudioClient);

void print_device_info(IMMDevice* pDevice, bool isDefault) {
    HRESULT hr;
    LPWSTR pwszID = NULL;
    IPropertyStore* pProps = NULL;
    PROPVARIANT varName;
    IAudioClient* pAudioClient = NULL;
    WAVEFORMATEX* pwfx = NULL;

    PropVariantInit(&varName);

    // Get device ID
    hr = pDevice->GetId(&pwszID);
    if (SUCCEEDED(hr)) {
        std::wcout << L"  Device ID: " << pwszID << std::endl;
        CoTaskMemFree(pwszID);
    }

    // Get device properties
    hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (SUCCEEDED(hr)) {
        // Get friendly name
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        if (SUCCEEDED(hr)) {
            std::wcout << L"  Name: " << varName.pwszVal;
            if (isDefault) {
                std::wcout << L" [DEFAULT]";
            }
            std::wcout << std::endl;
        }

        PropVariantClear(&varName);
        pProps->Release();
    }

    // Get audio format
    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (SUCCEEDED(hr)) {
        hr = pAudioClient->GetMixFormat(&pwfx);
        if (SUCCEEDED(hr)) {
            std::wcout << L"  Format: " << pwfx->nSamplesPerSec << L" Hz, "
                      << pwfx->nChannels << L" channels, "
                      << pwfx->wBitsPerSample << L" bits";
            
            if (pwfx->wFormatTag == 0x0003) {  // WAVE_FORMAT_IEEE_FLOAT
                std::wcout << L" (float)";
            } else if (pwfx->wFormatTag == 0x0001) {  // WAVE_FORMAT_PCM
                std::wcout << L" (PCM)";
            } else if (pwfx->wFormatTag == 0xFFFE) {  // WAVE_FORMAT_EXTENSIBLE
                std::wcout << L" (extensible)";
            }
            std::wcout << std::endl;

            CoTaskMemFree(pwfx);
        }
        pAudioClient->Release();
    }

    std::wcout << std::endl;
}

int main() {
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDeviceCollection* pCollection = NULL;
    IMMDevice* pDefaultDevice = NULL;
    UINT count;

    // Initialize COM
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return 1;
    }

    // Create device enumerator
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator" << std::endl;
        CoUninitialize();
        return 1;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "Windows Audio Capture Devices" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Get default device
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDefaultDevice);
    LPWSTR defaultID = NULL;
    if (SUCCEEDED(hr)) {
        pDefaultDevice->GetId(&defaultID);
    }

    // Get all capture devices
    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        std::cerr << "Failed to enumerate devices" << std::endl;
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }

    hr = pCollection->GetCount(&count);
    if (FAILED(hr)) {
        std::cerr << "Failed to get device count" << std::endl;
        pCollection->Release();
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }

    std::cout << "Found " << count << " active capture device(s):" << std::endl;
    std::cout << std::endl;

    for (UINT i = 0; i < count; i++) {
        IMMDevice* pDevice = NULL;
        
        hr = pCollection->Item(i, &pDevice);
        if (SUCCEEDED(hr)) {
            std::cout << "Device " << (i + 1) << ":" << std::endl;
            
            LPWSTR deviceID = NULL;
            pDevice->GetId(&deviceID);
            bool isDefault = (defaultID && deviceID && wcscmp(defaultID, deviceID) == 0);
            
            print_device_info(pDevice, isDefault);
            
            if (deviceID) CoTaskMemFree(deviceID);
            pDevice->Release();
        }
    }

    if (defaultID) CoTaskMemFree(defaultID);
    if (pDefaultDevice) pDefaultDevice->Release();
    pCollection->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;
}
