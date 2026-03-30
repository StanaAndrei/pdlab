#include <windows.h>
#include <initguid.h> 
#include <devpkey.h>
#include <setupapi.h>
#include <cfgmgr32.h> // Required for CM_DEVCAP_REMOVABLE
#include <iostream>
#include <vector>
#include <string>

// Link the SetupAPI library automatically for MSVC
#pragma comment(lib, "setupapi.lib")

void PrintDeviceProperty(HDEVINFO deviceInfoSet, PSP_DEVINFO_DATA deviceInfoData, const DEVPROPKEY* propKey, const wchar_t* label) {
    DEVPROPTYPE propType;
    DWORD requiredSize = 0;

    // 1. Find the required buffer size
    SetupDiGetDevicePropertyW(deviceInfoSet, deviceInfoData, propKey, &propType, nullptr, 0, &requiredSize, 0);

    if (requiredSize > 0) {
        std::vector<BYTE> buffer(requiredSize);

        // 2. Extract the property value
        if (SetupDiGetDevicePropertyW(deviceInfoSet, deviceInfoData, propKey, &propType, buffer.data(), requiredSize, nullptr, 0)) {

            // Handle standard string properties
            if (propType == DEVPROP_TYPE_STRING) {
                std::wcout << label << L": " << reinterpret_cast<wchar_t*>(buffer.data()) << std::endl;
            }
            // Handle lists of strings (like Hardware IDs)
            else if (propType == DEVPROP_TYPE_STRING_LIST) {
                std::wcout << label << L":" << std::endl;

                wchar_t* strPtr = reinterpret_cast<wchar_t*>(buffer.data());
                while (*strPtr != L'\0') {
                    std::wcout << L"      - " << strPtr << std::endl;
                    strPtr += wcslen(strPtr) + 1; // Move pointer to the next string in the list
                }
            }
        }
    }
}

int main() {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        std::cerr << "Error calling SetupDiGetClassDevs" << std::endl;
        return 1;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    std::wcout << L"--- Connected Physical Peripherals ---" << std::endl << std::endl;

    DWORD removableDeviceCount = 0;
    DWORD deviceIndex = 0;

    while (SetupDiEnumDeviceInfo(deviceInfoSet, deviceIndex, &deviceInfoData)) {
        deviceIndex++;

        // --- 1. FILTER BY REMOVABLE CAPABILITY ---
        DEVPROPTYPE capPropType;
        DWORD capReqSize = 0;
        UINT32 capabilities = 0;

        // Extract device capabilities
        if (SetupDiGetDevicePropertyW(deviceInfoSet, &deviceInfoData, &DEVPKEY_Device_Capabilities, &capPropType, (PBYTE)&capabilities, sizeof(capabilities), &capReqSize, 0)) {

            // Check if the device has the REMOVABLE flag (meaning it is an external peripheral)
            if ((capabilities & CM_DEVCAP_REMOVABLE) == 0) {
                continue; // Skip internal motherboard components
            }
        }
        else {
            continue; // Skip if we cannot read capabilities
        }

        // --- 2. EXTRACT ENUMERATOR (BUS) NAME ---
        DEVPROPTYPE enumPropType;
        DWORD enumReqSize = 0;
        std::wstring enumeratorName = L"Unknown";

        SetupDiGetDevicePropertyW(deviceInfoSet, &deviceInfoData, &DEVPKEY_Device_EnumeratorName, &enumPropType, nullptr, 0, &enumReqSize, 0);

        if (enumReqSize > 0) {
            std::vector<BYTE> enumBuffer(enumReqSize);
            if (SetupDiGetDevicePropertyW(deviceInfoSet, &deviceInfoData, &DEVPKEY_Device_EnumeratorName, &enumPropType, enumBuffer.data(), enumReqSize, nullptr, 0)) {
                enumeratorName = reinterpret_cast<wchar_t*>(enumBuffer.data());
            }
        }

        // Catch-all to filter out any virtual software devices that mistakenly claim to be removable
        if (enumeratorName == L"SWD" || enumeratorName == L"ROOT") {
            continue;
        }

        // --- 3. PRINT DEVICE METADATA ---
        removableDeviceCount++;
        std::wcout << L"Connected Device #" << removableDeviceCount << L" (Bus: " << enumeratorName << L")" << std::endl;

        PrintDeviceProperty(deviceInfoSet, &deviceInfoData, &DEVPKEY_Device_DeviceDesc, L"  Description");
        PrintDeviceProperty(deviceInfoSet, &deviceInfoData, &DEVPKEY_Device_FriendlyName, L"  Friendly Name");
        PrintDeviceProperty(deviceInfoSet, &deviceInfoData, &DEVPKEY_Device_Manufacturer, L"  Manufacturer");
        PrintDeviceProperty(deviceInfoSet, &deviceInfoData, &DEVPKEY_Device_HardwareIds, L"  Hardware IDs");

        std::wcout << L"---------------------------------------" << std::endl;
    }

    // Clean up resources
    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    std::wcout << L"Total connected peripherals found: " << removableDeviceCount << std::endl;

    return 0;
}

