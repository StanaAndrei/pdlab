#include <Windows.h>
#include <iostream>
#include <vector>
#include <iomanip>

#define MAX_NAME_SIZE (16383)
#define MAX_BUFF_SIZE (2048)


int main() {
    HKEY hKey;
    LPCWSTR subKey = L".DEFAULT\\Control Panel\\Desktop";

    LONG openStatus = RegOpenKeyExW(HKEY_USERS, subKey, 0, KEY_READ, &hKey);

    if (openStatus != ERROR_SUCCESS) {
        if (openStatus == ERROR_ACCESS_DENIED) {
            std::wcerr << L"Error: Access Denied! Try running Visual Studio as Administrator.\n";
        }
        else {
            std::wcerr << L"Error opening key. Error code: " << openStatus << std::endl;
        }
        return EXIT_FAILURE;
    }

    std::wcout << L"Success! Values in HKEY_USERS\\" << subKey << L":\n";
    std::wcout << L"----------------------------------------------------------\n";
    std::wcout << std::left << std::setw(30) << L"Value Name" << std::setw(15) << L"Type" << L"Data" << std::endl;
    std::wcout << L"----------------------------------------------------------\n";

    WCHAR valueName[MAX_NAME_SIZE];
    BYTE dataBuffer[MAX_BUFF_SIZE];

    DWORD i = 0;
    while (true) {
        DWORD valueNameSize = ARRAYSIZE(valueName);
        DWORD dataBufferSize = sizeof dataBuffer;
        DWORD type = 0;

        LONG result = RegEnumValueW(
            hKey,
            i,
            valueName,
            &valueNameSize,
            NULL,
            &type,
            dataBuffer,
            &dataBufferSize
        );

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if (result == ERROR_SUCCESS) {
            std::wcout << std::left << std::setw(30) << valueName;
            switch (type) {
            case REG_SZ: // String Type (Text)
                std::wcout << std::setw(15) << L"REG_SZ" << (LPCWSTR)dataBuffer << std::endl;
                break;
            case REG_DWORD: { // DWORD Type (32-bit number)
                DWORD val = *((DWORD*)dataBuffer);
                std::wcout << std::setw(15) << L"REG_DWORD" << val << L" (0x" << std::hex << val << std::dec << L")" << std::endl;
                break;
            }
            case REG_BINARY: // Binary Type (e.g., UserPreferencesMask)
                std::wcout << std::setw(15) << L"REG_BINARY" << L"[Binary Data]" << std::endl;
                break;
            default:
                std::wcout << std::setw(15) << L"Other Type" << L"---" << std::endl;
                break;
            }
        }
        i++;
    }

    RegCloseKey(hKey);
    return EXIT_SUCCESS;
}

