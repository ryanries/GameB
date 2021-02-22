// Platform-independent game code binary.

#pragma warning(push, 0)
#include <Windows.h>
#pragma warning(pop)

_declspec(dllexport) int TestFunc01(void)
{
    MessageBoxA(NULL, "Hello from TestFunc01", "Test", MB_OK | MB_ICONASTERISK);

    return(0);
}

// This DllMain intentionally does nothing.
BOOL WINAPI DllMain(HINSTANCE Instance, DWORD Reason, LPVOID Reserved)
{    
    UNREFERENCED_PARAMETER(Instance);

    UNREFERENCED_PARAMETER(Reserved);

    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            break;
        }
        case DLL_THREAD_ATTACH:
        {            
            break;
        }
        case DLL_THREAD_DETACH:
        {         
            break;
        }
        case DLL_PROCESS_DETACH:
        {         
            break;
        }
    }

    return(TRUE);
}