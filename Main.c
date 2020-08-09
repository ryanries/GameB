// Codename: GameB
// Will come up with a better name later.
// 2020 Joseph Ryan Ries <ryanries09@gmail.com>
// My YouTube series where we program an entire video game from scratch in C.
// Watch it on YouTube:    https://www.youtube.com/watch?v=3zFFrBSdBvA
// Follow along on GitHub: https://github.com/ryanries/GameB
// Find me on Twitter @JosephRyanRies 
//# License
//----------
//The source code in this project is licensed under the MIT license.
//The media assets such as artwork, custom fonts, music and sound effects are licensed under a separate license.
//A copy of that license can be found in the 'Assets' directory.

// --- TO DO ---
// 
// talk about the scope of #define precompiler directives
// disable keyboard input during gamestate fade-in
// shadow effect for text?
// should we enhance BlitStringToBuffer to support varargs?
// character naming screen
// overworld... tile maps...
// ogg vorbis background music
// archive format for game assets?
//
// Add logging to InitializeHero
//
// Add movement speed to player
//
//
//
// use enums instead of #defines ?
//
// LogMessageA option to spawn msgbox ? And assert ?
//
// Blit32bppbitmap draw off the edge of the screen ?
//
// Screen size code for monitors that are not 16 : 9
//
//
//
//
// Menus
// 
// Game states (Maybe each state should be a struct that includes an array of valid gamestates that it may transition to?)
//
// Tile maps

#define AVX                         // Valid options are SSE2, AVX, or nothing.

#pragma warning(push, 3)            // Temporarily reduce warning level for headers over which we have no control.

#include <stdio.h>                  // String manipulation functions such as sprintf, etc.

#include <windows.h>                // The primary header file for the Windows API

#include <psapi.h>                  // Process Status API, e.g. GetProcessMemoryInfo

#ifdef AVX

#include <immintrin.h>              // AVX (Advanced Vector Extensions)

#elif defined SSE2

#include <emmintrin.h>              // SSE2 (Streaming SIMD Extensions)

#endif

#include <xaudio2.h>                // Audio library

#pragma warning(pop)                // Restore warning level to /Wall

#include <Xinput.h>                 // Xbox 360 gamepad input

#include <stdint.h>                 // Nicer data types, e.g., uint8_t, int32_t, etc.

#include "Main.h"                   // The primary header file that defines stuff specific to our game.

#include "Menus.h"                  // Menus, menu items, etc.

#pragma comment(lib, "Winmm.lib")   // Windows Multimedia library, we use it for timeBeginPeriod to adjust the global system timer resolution.

#pragma comment(lib, "XAudio2.lib") // Audio library.

#pragma comment(lib, "XInput.lib")  // Xbox 360 gamepad input.


HWND gGameWindow;                   // A global handle to the game window.

BOOL gGameIsRunning;                // Set this to FALSE to exit the game immediately. This controls the main game loop in WinMain.

GAMEBITMAP gBackBuffer;             // The "drawing surface" which we blit to the screen once per frame, 60 times per second.

GAMEBITMAP g6x7Font;

// Map any char value to an offset dictated by the g6x7Font ordering.
int32_t gFontCharacterPixelOffset[] = {
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
    //     !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
        94,64,87,66,67,68,70,85,72,73,71,77,88,74,91,92,52,53,54,55,56,57,58,59,60,61,86,84,89,75,90,93,
    //  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
        65,0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,80,78,81,69,76,
    //  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  ..
        62,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,82,79,83,63,93,
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
    //  .. .. .. .. .. .. .. .. .. .. .. «  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. »  .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,96,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,95,93,93,93,93,
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. F2 .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,97,93,93,93,93,93,93,93,93,93,93,93,93,93
};

GAMEPERFDATA gPerformanceData;

HERO gPlayer;

BOOL gWindowHasFocus;

REGISTRYPARAMS gRegistryParams;

XINPUT_STATE gGamepadState;

int8_t gGamepadID = -1;

GAMESTATE gCurrentGameState = GAMESTATE_OPENINGSPLASHSCREEN;

GAMESTATE gPreviousGameState;

GAMESTATE gDesiredGameState;

GAMEINPUT gGameInput;

IXAudio2* gXAudio;

IXAudio2MasteringVoice* gXAudioMasteringVoice;

IXAudio2SourceVoice* gXAudioSFXSourceVoice[NUMBER_OF_SFX_SOURCE_VOICES];

IXAudio2SourceVoice* gXAudioMusicSourceVoice;

uint8_t gSFXSourceVoiceSelector;

float gSFXVolume = 0.5f;

float gMusicVolume = 0.5f;

GAMESOUND gSoundMenuNavigate;

GAMESOUND gSoundMenuChoose;

GAMESOUND gSoundSplashScreen;



int __stdcall WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ PSTR CommandLine, _In_ INT CmdShow)
{
    UNREFERENCED_PARAMETER(Instance);

	UNREFERENCED_PARAMETER(PreviousInstance);

	UNREFERENCED_PARAMETER(CommandLine);

	UNREFERENCED_PARAMETER(CmdShow);    

    MSG Message = { 0 };

    int64_t FrameStart = 0;

    int64_t FrameEnd = 0;

    int64_t ElapsedMicroseconds = 0;

    int64_t ElapsedMicrosecondsAccumulatorRaw = 0;

    int64_t ElapsedMicrosecondsAccumulatorCooked = 0;    

    HMODULE NtDllModuleHandle = NULL;

    FILETIME ProcessCreationTime = { 0 };

    FILETIME ProcessExitTime = { 0 };

    int64_t CurrentUserCPUTime = 0;

    int64_t CurrentKernelCPUTime = 0;

    int64_t PreviousUserCPUTime = 0;

    int64_t PreviousKernelCPUTime = 0;

    HANDLE ProcessHandle = GetCurrentProcess();

    if (LoadRegistryParameters() != ERROR_SUCCESS)
    {
        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] %s %s is starting.", __FUNCTION__, GAME_NAME, GAME_VER);

    if (GameIsAlreadyRunning() == TRUE)
    {
        LogMessageA(LL_ERROR, "[%s] Another instance of this program is already running!", __FUNCTION__);

        MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((NtDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL)
    {
        LogMessageA(LL_ERROR, "[%s] Couldn't load ntdll.dll! Error 0x%08lx!", __FUNCTION__, GetLastError());

        MessageBoxA(NULL, "Couldn't load ntdll.dll!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(NtDllModuleHandle, "NtQueryTimerResolution")) == NULL)
    {
        LogMessageA(LL_ERROR, "[%s] Couldn't find the NtQueryTimerResolution function in ntdll.dll! GetProcAddress failed! Error 0x%08lx!", __FUNCTION__, GetLastError());

        MessageBoxA(NULL, "Couldn't find the NtQueryTimerResolution function in ntdll.dll!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    NtQueryTimerResolution(&gPerformanceData.MinimumTimerResolution, &gPerformanceData.MaximumTimerResolution, &gPerformanceData.CurrentTimerResolution);

    GetSystemInfo(&gPerformanceData.SystemInfo);

    LogMessageA(LL_INFO, "[%s] Number of CPUs: %d", __FUNCTION__, gPerformanceData.SystemInfo.dwNumberOfProcessors);

    switch (gPerformanceData.SystemInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_INTEL:
        {
            LogMessageA(LL_INFO, "[%s] CPU Architecture: x86", __FUNCTION__);

            break;
        }
        case PROCESSOR_ARCHITECTURE_IA64:
        {
            LogMessageA(LL_INFO, "[%s] CPU Architecture: Itanium", __FUNCTION__);

            break;
        }
        case PROCESSOR_ARCHITECTURE_ARM64:
        {
            LogMessageA(LL_INFO, "[%s] CPU Architecture: ARM64", __FUNCTION__);

            break;
        }
        case PROCESSOR_ARCHITECTURE_ARM:
        {
            LogMessageA(LL_INFO, "[%s] CPU Architecture: ARM", __FUNCTION__);

            break;
        }
        case PROCESSOR_ARCHITECTURE_AMD64:
        {
            LogMessageA(LL_INFO, "[%s] CPU Architecture: x64", __FUNCTION__);

            break;
        }
        default:
        {
            LogMessageA(LL_INFO, "[%s] CPU Architecture: Unknown", __FUNCTION__);
        }
    }

    GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.PreviousSystemTime);

    if (timeBeginPeriod(1) == TIMERR_NOCANDO)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set global timer resolution!", __FUNCTION__);

        MessageBoxA(NULL, "Failed to set global timer resolution!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] System Timer Resolution: Min %d Max %d Current %d",
        __FUNCTION__,
        gPerformanceData.MinimumTimerResolution,
        gPerformanceData.MaximumTimerResolution,
        gPerformanceData.CurrentTimerResolution);

    if (SetPriorityClass(ProcessHandle, HIGH_PRIORITY_CLASS) == 0)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set process priority! Error 0x%08lx", __FUNCTION__, GetLastError());

        MessageBoxA(NULL, "Failed to set process priority!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == 0)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set thread priority! Error 0x%08lx!", __FUNCTION__, GetLastError());

        MessageBoxA(NULL, "Failed to set thread priority!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] CreateMainGameWindow failed!", __FUNCTION__);

        goto Exit;
    }

    if ((Load32BppBitmapFromFile(".\\Assets\\6x7Font.bmpx", &g6x7Font)) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading 6x7font.bmpx failed!", __FUNCTION__);

        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (InitializeSoundEngine() != S_OK)
    {
        MessageBoxA(NULL, "InitializeSoundEngine failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadWavFromFile(".\\Assets\\SplashScreen.wav", &gSoundSplashScreen) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWavFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadWavFromFile(".\\Assets\\MenuNavigate.wav", &gSoundMenuNavigate) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWavFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadWavFromFile(".\\Assets\\MenuChoose.wav", &gSoundMenuChoose) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWavFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }
    
    QueryPerformanceFrequency((LARGE_INTEGER*)&gPerformanceData.PerfFrequency);

    gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);

    gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;

    gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;

    gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;

    gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;

    gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;

    gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.Memory == NULL)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to allocate memory for drawing surface! Error 0x%08lx!", __FUNCTION__, ERROR_NOT_ENOUGH_MEMORY);

        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    memset(gBackBuffer.Memory, 0x7F, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (InitializeHero() != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to initialize hero!", __FUNCTION__);

        MessageBoxA(NULL, "Failed to initialize hero!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    gGameIsRunning = TRUE;

    while (gGameIsRunning == TRUE)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&FrameStart);

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

        ElapsedMicroseconds = FrameEnd - FrameStart;

        ElapsedMicroseconds *= 1000000;

        ElapsedMicroseconds /= gPerformanceData.PerfFrequency;

        gPerformanceData.TotalFramesRendered++;

        ElapsedMicrosecondsAccumulatorRaw += ElapsedMicroseconds;

        while (ElapsedMicroseconds < TARGET_MICROSECONDS_PER_FRAME)
        {
            ElapsedMicroseconds = FrameEnd - FrameStart;

            ElapsedMicroseconds *= 1000000;

            ElapsedMicroseconds /= gPerformanceData.PerfFrequency;

            QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

            // If we are less than 75% of the way through the current frame, then rest.

            if (ElapsedMicroseconds < (TARGET_MICROSECONDS_PER_FRAME * 0.75f))
            {
                // Could be anywhere from 1ms to a full system timer tick? (~15.625ms)
                Sleep(1);
            }
        }

        ElapsedMicrosecondsAccumulatorCooked += ElapsedMicroseconds;

        if ((gPerformanceData.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES) == 0)
        {
            GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.CurrentSystemTime);

            GetProcessTimes(ProcessHandle,
                &ProcessCreationTime, 
                &ProcessExitTime, 
                (FILETIME*)&CurrentKernelCPUTime,
                (FILETIME*)&CurrentUserCPUTime);

            gPerformanceData.CPUPercent = (double)(CurrentKernelCPUTime - PreviousKernelCPUTime) + (CurrentUserCPUTime - PreviousUserCPUTime);

            gPerformanceData.CPUPercent /= (gPerformanceData.CurrentSystemTime - gPerformanceData.PreviousSystemTime);

            gPerformanceData.CPUPercent /= gPerformanceData.SystemInfo.dwNumberOfProcessors;

            gPerformanceData.CPUPercent *= 100;

            GetProcessHandleCount(ProcessHandle, &gPerformanceData.HandleCount);

            K32GetProcessMemoryInfo(ProcessHandle, (PROCESS_MEMORY_COUNTERS*)&gPerformanceData.MemInfo, sizeof(gPerformanceData.MemInfo));

            gPerformanceData.RawFPSAverage = 1.0f / ((ElapsedMicrosecondsAccumulatorRaw / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);

            gPerformanceData.CookedFPSAverage = 1.0f / ((ElapsedMicrosecondsAccumulatorCooked / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);

            FindFirstConnectedGamepad();

            ElapsedMicrosecondsAccumulatorRaw = 0;

            ElapsedMicrosecondsAccumulatorCooked = 0;

            PreviousKernelCPUTime = CurrentKernelCPUTime;

            PreviousUserCPUTime = CurrentUserCPUTime;            

            gPerformanceData.PreviousSystemTime = gPerformanceData.CurrentSystemTime;
        }        
    }


Exit:

    LogMessageA(LL_INFO, "Game is exiting.\r\n");

	return(0);
}

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CLOSE:
        {
            gGameIsRunning = FALSE;

            PostQuitMessage(0);

            break;
        }
        case WM_ACTIVATE:
        {
            if (WParam == 0)
            {
                // Our window has lost focus

                gWindowHasFocus = FALSE;
            }
            else
            {
                // Our window has gained focus

                ShowCursor(FALSE);

                gWindowHasFocus = TRUE;
            }

            break;
        }
        default:
        {
            Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }
    }

    return(Result);
}

// TODO: Consider non-16:9 displays. E.g., ultra-wide monitors will have to have black bars on the sides, with the game center screen.
DWORD CreateMainGameWindow(void)
{
    DWORD Result = ERROR_SUCCESS;

    WNDCLASSEXA WindowClass = { 0 };

    WindowClass.cbSize = sizeof(WNDCLASSEXA);

    WindowClass.style = 0;

    WindowClass.lpfnWndProc = MainWindowProc;

    WindowClass.cbClsExtra = 0;

    WindowClass.cbWndExtra = 0;

    WindowClass.hInstance = GetModuleHandleA(NULL);

    WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
#ifdef _DEBUG
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
#else
    WindowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
#endif

    WindowClass.lpszMenuName = NULL;

    WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";   

    // This is set in the application manifest, as recommended by MSDN.
    // SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();

        LogMessageA(LL_ERROR, "[%s] RegisterClassExA failed! Error 0x%08lx!", __FUNCTION__, Result);

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, WindowClass.lpszClassName, GAME_NAME, WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, GetModuleHandleA(NULL), NULL);

    if (gGameWindow == NULL)
    {
        Result = GetLastError();

        LogMessageA(LL_ERROR, "[%s] CreateWindowExA failed! Error 0x%08lx!", __FUNCTION__, Result);

        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    gPerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.MonitorInfo) == 0)
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        LogMessageA(LL_ERROR, "[%s] GetMonitorInfoA(MonitorFromWindow()) failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    for (uint8_t Counter = 1; Counter < 12; Counter++)
    {
        if (GAME_RES_WIDTH * Counter > (gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left) ||
            GAME_RES_HEIGHT * Counter > (gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top))
        {
            gPerformanceData.MaxScaleFactor = Counter - 1;

            break;
        }
    }

    if (gRegistryParams.ScaleFactor == 0)
    {
        gPerformanceData.CurrentScaleFactor = gPerformanceData.MaxScaleFactor;
    }
    else
    {
        gPerformanceData.CurrentScaleFactor = (uint8_t)gRegistryParams.ScaleFactor;
    }

    // This can happen if for example the user has a laptop with a smaller screen,
    // then docks the laptop into a docking station with a larger screen, then undocks
    // the laptop again and plays on the smaller screen.

    if (gPerformanceData.CurrentScaleFactor > gPerformanceData.MaxScaleFactor)
    {
        gPerformanceData.CurrentScaleFactor = gPerformanceData.MaxScaleFactor;

        LogMessageA(LL_WARNING, "[%s] ScaleFactor read from the registry was larger than the maximum allowable scale factor according to your current monitor size. Resetting CurrentScaleFactor to match MaxScaleFactor.", __FUNCTION__);
    }

    LogMessageA(LL_INFO, "[%s] Current scale factor is %d. Max scale factor is %d. ", 
        __FUNCTION__, 
        gPerformanceData.CurrentScaleFactor, 
        gPerformanceData.MaxScaleFactor);

    LogMessageA(LL_INFO, "[%s] Will draw at %dx%d.", 
        __FUNCTION__, 
        GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor,
        GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor);

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, WS_VISIBLE) == 0)
    {
        Result = GetLastError();

        LogMessageA(LL_ERROR, "[%s] SetWindowLongPtrA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }     
        
    if (SetWindowPos(gGameWindow,
        HWND_TOP,
        gPerformanceData.MonitorInfo.rcMonitor.left,
        gPerformanceData.MonitorInfo.rcMonitor.top,
        gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left,
        gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top,
        SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
    {
        Result = GetLastError();

        LogMessageA(LL_ERROR, "[%s] SetWindowPos failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

Exit:

    return(Result);
}

BOOL GameIsAlreadyRunning(void)
{
    HANDLE Mutex = NULL;

    Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

void ProcessPlayerInput(void)
{
    if (gWindowHasFocus == FALSE)
    {
        return;
    }

    gGameInput.EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    gGameInput.DebugKeyIsDown  = GetAsyncKeyState(VK_F1);

    gGameInput.LeftKeyIsDown   = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');

    gGameInput.RightKeyIsDown  = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');

    gGameInput.UpKeyIsDown     = GetAsyncKeyState(VK_UP) | GetAsyncKeyState('W');

    gGameInput.DownKeyIsDown   = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');

    gGameInput.ChooseKeyIsDown = GetAsyncKeyState(VK_RETURN);

    if (gGamepadID > -1)
    {
        if (XInputGetState(gGamepadID, &gGamepadState) == ERROR_SUCCESS)
        {
            gGameInput.EscapeKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;

            gGameInput.LeftKeyIsDown   |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;

            gGameInput.RightKeyIsDown  |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

            gGameInput.UpKeyIsDown     |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;

            gGameInput.DownKeyIsDown   |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;

            gGameInput.ChooseKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_A;
        }
        else
        {
            // Gamepad unplugged?

            gGamepadID = -1;

            gPreviousGameState = gCurrentGameState;

            gCurrentGameState = GAMESTATE_GAMEPADUNPLUGGED;
        }
    }

    if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown)
    {
        gPerformanceData.DisplayDebugInfo = !gPerformanceData.DisplayDebugInfo;
    }

    switch (gCurrentGameState)
    {
        case GAMESTATE_OPENINGSPLASHSCREEN:
        {
            PPI_OpeningSplashScreen();

            break;
        }
        case GAMESTATE_GAMEPADUNPLUGGED:
        {
            PPI_GamepadUnplugged();

            break;
        }
        case GAMESTATE_TITLESCREEN:
        {
            PPI_TitleScreen();

            break;
        }
        case GAMESTATE_OVERWORLD:
        {
            PPI_Overworld();

            break;
        }
        case GAMESTATE_BATTLE:
        {
            break;
        }
        case GAMESTATE_CHARACTERNAMING:
        {
            PPI_CharacterNaming();

            break;
        }
        case GAMESTATE_OPTIONSSCREEN:
        {
            PPI_OptionsScreen();

            break;
        }
        case GAMESTATE_EXITYESNOSCREEN:
        {
            PPI_ExitYesNo();

            break;
        }
        default:
        {
            ASSERT(FALSE, "Unknown game state!");
        }
    }

    gGameInput.DebugKeyWasDown  = gGameInput.DebugKeyIsDown;

    gGameInput.LeftKeyWasDown   = gGameInput.LeftKeyIsDown;

    gGameInput.RightKeyWasDown  = gGameInput.RightKeyIsDown;

    gGameInput.UpKeyWasDown     = gGameInput.UpKeyIsDown;

    gGameInput.DownKeyWasDown   = gGameInput.DownKeyIsDown;

    gGameInput.ChooseKeyWasDown = gGameInput.ChooseKeyIsDown;

    gGameInput.EscapeKeyWasDown = gGameInput.EscapeKeyIsDown;
}

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap)
{
    DWORD Error = ERROR_SUCCESS;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    WORD BitmapHeader = 0;

    DWORD PixelDataOffset = 0;

    DWORD NumberOfBytesRead = 2;

    if ((FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();

        goto Exit;
    }
    
    if (ReadFile(FileHandle, &BitmapHeader, 2, &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (BitmapHeader != 0x4d42) // "BM" backwards
    {
        Error = ERROR_FILE_INVALID;

        goto Exit;
    }

    if (SetFilePointer(FileHandle, 0xA, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (ReadFile(FileHandle, &PixelDataOffset, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (SetFilePointer(FileHandle, 0xE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (ReadFile(FileHandle, &GameBitmap->BitmapInfo.bmiHeader, sizeof(BITMAPINFOHEADER), &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }

    if ((GameBitmap->Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GameBitmap->BitmapInfo.bmiHeader.biSizeImage)) == NULL)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;

        goto Exit;
    }

    if (SetFilePointer(FileHandle, PixelDataOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (ReadFile(FileHandle, GameBitmap->Memory, GameBitmap->BitmapInfo.bmiHeader.biSizeImage, &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }    

Exit:

    if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
    {
        CloseHandle(FileHandle);
    }

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Loading successful: %s", __FUNCTION__, FileName);
    }
    else
    {
        LogMessageA(LL_ERROR, "[%s] Loading failed: %s! Error 0x%08lx!", __FUNCTION__, FileName, Error);
    }

    return(Error);
}

DWORD InitializeHero(void)
{
    DWORD Error = ERROR_SUCCESS;

    gPlayer.ScreenPosX = 192;

    gPlayer.ScreenPosY = 64;

    gPlayer.CurrentArmor = SUIT_0;

    gPlayer.Direction = DOWN;

    

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Down_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Down_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Down_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Left_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Left_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Left_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Right_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Right_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Right_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Up_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Up_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Up_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

Exit:

    return(Error);
}

void BlitStringToBuffer(_In_ char* String, _In_ GAMEBITMAP* FontSheet, _In_ PIXEL32* Color, _In_ uint16_t x, _In_ uint16_t y)
{
    uint16_t CharWidth = (uint16_t)FontSheet->BitmapInfo.bmiHeader.biWidth / FONT_SHEET_CHARACTERS_PER_ROW;

    uint16_t CharHeight = (uint16_t)FontSheet->BitmapInfo.bmiHeader.biHeight;

    uint16_t BytesPerCharacter = (CharWidth * CharHeight * (FontSheet->BitmapInfo.bmiHeader.biBitCount / 8));

    uint16_t StringLength = (uint16_t)strlen(String);

    GAMEBITMAP StringBitmap = { 0 };

    StringBitmap.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;

    StringBitmap.BitmapInfo.bmiHeader.biHeight = CharHeight;

    StringBitmap.BitmapInfo.bmiHeader.biWidth = CharWidth * StringLength;

    StringBitmap.BitmapInfo.bmiHeader.biPlanes = 1;

    StringBitmap.BitmapInfo.bmiHeader.biCompression = BI_RGB;

    StringBitmap.Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ((size_t)BytesPerCharacter * (size_t)StringLength));

    for (int Character = 0; Character < StringLength; Character++)
    {
        int StartingFontSheetPixel = 0;

        int FontSheetOffset = 0;

        int StringBitmapOffset = 0;

        PIXEL32 FontSheetPixel = { 0 };

        StartingFontSheetPixel = (FontSheet->BitmapInfo.bmiHeader.biWidth * FontSheet->BitmapInfo.bmiHeader.biHeight) - \
            FontSheet->BitmapInfo.bmiHeader.biWidth + (CharWidth * gFontCharacterPixelOffset[(uint8_t)String[Character]]);

        for (int YPixel = 0; YPixel < CharHeight; YPixel++)
        {
            for (int XPixel = 0; XPixel < CharWidth; XPixel++)
            {
                FontSheetOffset = StartingFontSheetPixel + XPixel - (FontSheet->BitmapInfo.bmiHeader.biWidth * YPixel);

                StringBitmapOffset = (Character * CharWidth) + ((StringBitmap.BitmapInfo.bmiHeader.biWidth * StringBitmap.BitmapInfo.bmiHeader.biHeight) - \
                    StringBitmap.BitmapInfo.bmiHeader.biWidth) + XPixel - (StringBitmap.BitmapInfo.bmiHeader.biWidth) * YPixel;

                memcpy_s(&FontSheetPixel, sizeof(PIXEL32), (PIXEL32*)FontSheet->Memory + FontSheetOffset, sizeof(PIXEL32));

                FontSheetPixel.Red   = Color->Red;

                FontSheetPixel.Green = Color->Green;

                FontSheetPixel.Blue  = Color->Blue;

                memcpy_s((PIXEL32*)StringBitmap.Memory + StringBitmapOffset, sizeof(PIXEL32), &FontSheetPixel, sizeof(PIXEL32));

            }
        }
    }

    Blit32BppBitmapToBuffer(&StringBitmap, x, y);

    if (StringBitmap.Memory)
    {
        HeapFree(GetProcessHeap(), 0, StringBitmap.Memory);
    }
}

void RenderFrameGraphics(void)
{
    switch (gCurrentGameState)
    {
        case GAMESTATE_OPENINGSPLASHSCREEN:
        {
            DrawOpeningSplashScreen();

            break;
        }
        case GAMESTATE_GAMEPADUNPLUGGED:
        {
            DrawGamepadUnplugged();

            break;
        }
        case GAMESTATE_TITLESCREEN:
        {
            DrawTitleScreen();

            break;
        }
        case GAMESTATE_CHARACTERNAMING:
        {
            DrawCharacterNaming();

            break;
        }
        case GAMESTATE_OVERWORLD:
        {
            //DrawOverworld();

            break;
        }
        case GAMESTATE_BATTLE:
        {
            break;
        }
        case GAMESTATE_EXITYESNOSCREEN:
        {
            DrawExitYesNoScreen();

            break;
        }
        case GAMESTATE_OPTIONSSCREEN:
        {
            DrawOptionsScreen();

            break;
        }
        default:
        {
            ASSERT(FALSE, "Gamestate not implemented!");
        }
    }

    if (gPerformanceData.DisplayDebugInfo == TRUE)
    {
        DrawDebugInfo();
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext, 
        ((gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left) / 2) - ((GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor) / 2),
        ((gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top) / 2) - ((GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor) / 2),
        GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor,
        GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor,
        0, 
        0, 
        GAME_RES_WIDTH, 
        GAME_RES_HEIGHT, 
        gBackBuffer.Memory, 
        &gBackBuffer.BitmapInfo, 
        DIB_RGB_COLORS, 
        SRCCOPY);

    ReleaseDC(gGameWindow, DeviceContext);
}


#ifdef AVX

__forceinline void ClearScreen(_In_ __m256i* Color)
{
    for (int Index = 0; Index < (GAME_RES_WIDTH * GAME_RES_HEIGHT) / (sizeof(__m256i) / sizeof(PIXEL32)); Index++)
    {
        _mm256_store_si256((__m256i*)gBackBuffer.Memory + Index, *Color);
    }
}

#elif defined SSE2

__forceinline void ClearScreen(_In_ __m128i* Color)
{
    for (int Index = 0; Index < (GAME_RES_WIDTH * GAME_RES_HEIGHT) / (sizeof(__m128i) / sizeof(PIXEL32)); Index++)
    {
        _mm_store_si128((__m128i*)gBackBuffer.Memory + Index, *Color);
    }
}

#else

__forceinline void ClearScreen(_In_ PIXEL32* Pixel)
{
    for (int Index = 0; Index < GAME_RES_WIDTH * GAME_RES_HEIGHT; Index++)
    {
        memcpy((PIXEL32*)gBackBuffer.Memory + Index, Pixel, sizeof(PIXEL32));
    }
}

#endif

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y)
{
    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * y) + x;

    int32_t StartingBitmapPixel = ((GameBitmap->BitmapInfo.bmiHeader.biWidth * GameBitmap->BitmapInfo.bmiHeader.biHeight) - \
        GameBitmap->BitmapInfo.bmiHeader.biWidth);

    int32_t MemoryOffset = 0;

    int32_t BitmapOffset = 0;

    PIXEL32 BitmapPixel = { 0 };    

    for (int16_t YPixel = 0; YPixel < GameBitmap->BitmapInfo.bmiHeader.biHeight; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GameBitmap->BitmapInfo.bmiHeader.biWidth; XPixel++)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            if (BitmapPixel.Alpha == 255)
            {
                memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
            }
        }
    }    
}

DWORD LoadRegistryParameters(void)
{
    DWORD Result = ERROR_SUCCESS;

    HKEY RegKey = NULL;

    DWORD RegDisposition = 0;

    DWORD RegBytesRead = sizeof(DWORD);

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\" GAME_NAME, 0, NULL, 0, KEY_READ, NULL, &RegKey, &RegDisposition);

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (RegDisposition == REG_CREATED_NEW_KEY)
    {
        LogMessageA(LL_INFO, "[%s] Registry key did not exist; created new key HKCU\\SOFTWARE\\%s.", __FUNCTION__, GAME_NAME);
    }
    else
    {
        LogMessageA(LL_INFO, "[%s] Opened existing registry key HCKU\\SOFTWARE\\%s", __FUNCTION__, GAME_NAME);
    }

    Result = RegGetValueA(RegKey, NULL, "LogLevel", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.LogLevel, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(LL_INFO, "[%s] Registry value 'LogLevel' not found. Using default of 0. (LOG_LEVEL_NONE)", __FUNCTION__);

            gRegistryParams.LogLevel = LL_NONE;
        }
        else
        {
            LogMessageA(LL_ERROR, "[%s] Failed to read the 'LogLevel' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    Result = RegGetValueA(RegKey, NULL, "ScaleFactor", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.ScaleFactor, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(LL_INFO, "[%s] Registry value 'ScaleFactor' not found. Using default of 0.", __FUNCTION__);

            gRegistryParams.ScaleFactor = 0;
        }
        else
        {
            LogMessageA(LL_ERROR, "[%s] Failed to read the 'ScaleFactor' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(LL_INFO, "[%s] ScaleFactor is %d.", __FUNCTION__, gRegistryParams.ScaleFactor);    

    Result = RegGetValueA(RegKey, NULL, "SFXVolume", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.SFXVolume, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(LL_INFO, "[%s] Registry value 'SFXVolume' not found. Using default of 0.5/50.", __FUNCTION__);

            gRegistryParams.SFXVolume = 50;
        }
        else
        {
            LogMessageA(LL_ERROR, "[%s] Failed to read the 'SFXVolume' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(LL_INFO, "[%s] SFXVolume is %.1f/%d.", __FUNCTION__, (float)(gRegistryParams.SFXVolume / 100.0f), gRegistryParams.SFXVolume);

    gSFXVolume = (float)(gRegistryParams.SFXVolume / 100.0f);

    Result = RegGetValueA(RegKey, NULL, "MusicVolume", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.MusicVolume, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(LL_INFO, "[%s] Registry value 'MusicVolume' not found. Using default of 0.5/50.", __FUNCTION__);

            gRegistryParams.MusicVolume = 50;
        }
        else
        {
            LogMessageA(LL_ERROR, "[%s] Failed to read the 'MusicVolume' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(LL_INFO, "[%s] MusicVolume is %.1f/%d.", __FUNCTION__, (float)(gRegistryParams.MusicVolume / 100.0f), gRegistryParams.MusicVolume);

    gMusicVolume = (float)(gRegistryParams.MusicVolume / 100.0f);

Exit:

    if (RegKey)
    {
        RegCloseKey(RegKey);
    }

    return(Result);
}

DWORD SaveRegistryParameters(void)
{
    DWORD Result = ERROR_SUCCESS;

    HKEY RegKey = NULL;

    DWORD RegDisposition = 0;    

    DWORD SFXVolume = (DWORD)(gSFXVolume * 100.0f);
    
    DWORD MusicVolume = (DWORD)(gMusicVolume * 100.0f);

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\" GAME_NAME, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &RegKey, &RegDisposition);

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] Reg key open for save.", __FUNCTION__);

    Result = RegSetValueExA(RegKey, "SFXVolume", 0, REG_DWORD, (const BYTE*)&SFXVolume, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set 'SFXVolume' in the registry! Error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] SFXVolume saved: %d.", __FUNCTION__, SFXVolume);

    Result = RegSetValueExA(RegKey, "MusicVolume", 0, REG_DWORD, (const BYTE*)&MusicVolume, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set 'MusicVolume' in the registry! Error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] MusicVolume saved: %d.", __FUNCTION__, MusicVolume);

    Result = RegSetValueExA(RegKey, "ScaleFactor", 0, REG_DWORD, &gPerformanceData.CurrentScaleFactor, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set 'ScaleFactor' in the registry! Error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] ScaleFactor saved: %d.", __FUNCTION__, gPerformanceData.CurrentScaleFactor);


Exit:

    if (RegKey)
    {
        RegCloseKey(RegKey);
    }

    return(Result);
}

void LogMessageA(_In_ LOGLEVEL LogLevel, _In_ char* Message, _In_ ...)
{
    size_t MessageLength = strlen(Message);

    SYSTEMTIME Time = { 0 };

    HANDLE LogFileHandle = INVALID_HANDLE_VALUE;

    DWORD EndOfFile = 0;

    DWORD NumberOfBytesWritten = 0;

    char DateTimeString[96] = { 0 };

    char SeverityString[8] = { 0 };

    char FormattedString[4096] = { 0 };
    

    if (gRegistryParams.LogLevel < (DWORD)LogLevel)
    {
        return;
    }

    if (MessageLength < 1 || MessageLength > 4095)
    {
        ASSERT(FALSE, "Message was either too short or too long!");
    }

    switch (LogLevel)
    {
        case LL_NONE:
        {
            return;
        }
        case LL_INFO:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[INFO]");

            break;
        }
        case LL_WARNING:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[WARN]");

            break;
        }
        case LL_ERROR:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[ERROR]");

            break;
        }
        case LL_DEBUG:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[DEBUG]");
        }
        default:
        {
            ASSERT(FALSE, "Unrecognized log level!");
        }
    }

    GetLocalTime(&Time);

    va_list ArgPointer = NULL;

    va_start(ArgPointer, Message);

    _vsnprintf_s(FormattedString, sizeof(FormattedString), _TRUNCATE, Message, ArgPointer);

    va_end(ArgPointer);

    _snprintf_s(DateTimeString, sizeof(DateTimeString), _TRUNCATE, "\r\n[%02u/%02u/%u %02u:%02u:%02u.%03u]", Time.wMonth, Time.wDay, Time.wYear, Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds);

    if ((LogFileHandle = CreateFileA(LOG_FILE_NAME, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        ASSERT(FALSE, "Failed to access log file!");        
    }

    EndOfFile = SetFilePointer(LogFileHandle, 0, NULL, FILE_END);

    WriteFile(LogFileHandle, DateTimeString, (DWORD)strlen(DateTimeString), &NumberOfBytesWritten, NULL);

    WriteFile(LogFileHandle, SeverityString, (DWORD)strlen(SeverityString), &NumberOfBytesWritten, NULL);

    WriteFile(LogFileHandle, FormattedString, (DWORD)strlen(FormattedString), &NumberOfBytesWritten, NULL);

    if (LogFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(LogFileHandle);
    }
}

__forceinline void DrawDebugInfo(void)
{
    char DebugTextBuffer[64] = { 0 };

    PIXEL32 White = { 0xFF, 0xFF, 0xFF, 0xFF };

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPSRaw:  %.01f", gPerformanceData.RawFPSAverage);

    //BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &(PIXEL32) { 0x60, 0x60, 0x60, 0xff }, 1, 1);
    
    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 0);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPSCookd:%.01f", gPerformanceData.CookedFPSAverage);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 8);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "MinTimer:%.02f", gPerformanceData.MinimumTimerResolution / 10000.0f);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 16);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "MaxTimer:%.02f", gPerformanceData.MaximumTimerResolution / 10000.0f);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 24);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CurTimer:%.02f", gPerformanceData.CurrentTimerResolution / 10000.0f);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 32);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Handles: %lu", gPerformanceData.HandleCount);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 40);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Memory:  %zu KB", gPerformanceData.MemInfo.PrivateUsage / 1024);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 48);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CPU:     %.02f%%", gPerformanceData.CPUPercent);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 56);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FramesT: %llu", gPerformanceData.TotalFramesRendered);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 64);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "ScreenXY:%d,%d", gPlayer.ScreenPosX, gPlayer.ScreenPosY);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 72);
}

void FindFirstConnectedGamepad(void)
{
    gGamepadID = -1;

    for (int8_t GamepadIndex = 0; GamepadIndex < XUSER_MAX_COUNT && gGamepadID == -1; GamepadIndex++)
    {
        XINPUT_STATE State = { 0 };

        if (XInputGetState(GamepadIndex, &State) == ERROR_SUCCESS)
        {
            gGamepadID = GamepadIndex;
        }
    }
}

void MenuItem_TitleScreen_Resume(void)
{

}

void MenuItem_TitleScreen_StartNew(void)
{
    // If a game is already in progress, this should prompt the user if they are sure that they want to start a new game first,
    // and lose any unsaved progress.
    // Otherwise, just go to the character naming screen.

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_CHARACTERNAMING;
}

void MenuItem_CharacterNaming_Add(void)
{
    if (strlen(gPlayer.Name) < 8)
    {
        gPlayer.Name[strlen(gPlayer.Name)] = gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->Name[0];

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_CharacterNaming_Back(void)
{
    if (strlen(gPlayer.Name) < 1)
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_TITLESCREEN;
    }
    else
    {
        gPlayer.Name[strlen(gPlayer.Name) - 1] = '\0';
    }

    PlayGameSound(&gSoundMenuChoose);
}

void MenuItem_CharacterNaming_OK(void)
{

}

void MenuItem_TitleScreen_Options(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_OPTIONSSCREEN;
}

void MenuItem_TitleScreen_Exit(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_EXITYESNOSCREEN;    
}

void MenuItem_ExitYesNo_Yes(void)
{
    SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
}

void MenuItem_ExitYesNo_No(void)
{
    gCurrentGameState = gPreviousGameState;

    gPreviousGameState = GAMESTATE_EXITYESNOSCREEN;
}

void MenuItem_OptionsScreen_SFXVolume(void)
{
    gSFXVolume += 0.1f;

    if ((uint8_t)(gSFXVolume * 10) > 10)
    {
        gSFXVolume = 0.0f;
    }

    for (uint8_t Counter = 0; Counter < NUMBER_OF_SFX_SOURCE_VOICES; Counter++)
    {
        gXAudioSFXSourceVoice[Counter]->lpVtbl->SetVolume(gXAudioSFXSourceVoice[Counter], gSFXVolume, XAUDIO2_COMMIT_NOW);
    }
}

void MenuItem_OptionsScreen_MusicVolume(void)
{
    gMusicVolume += 0.1f;

    if ((uint8_t)(gMusicVolume * 10) > 10)
    {
        gMusicVolume = 0.0f;
    }   
        
    gXAudioMusicSourceVoice->lpVtbl->SetVolume(gXAudioMusicSourceVoice, gMusicVolume, XAUDIO2_COMMIT_NOW);    
}

void MenuItem_OptionsScreen_ScreenSize(void)
{
    if (gPerformanceData.CurrentScaleFactor < gPerformanceData.MaxScaleFactor)
    {
        gPerformanceData.CurrentScaleFactor++;
    }
    else
    {
        gPerformanceData.CurrentScaleFactor = 1;
    }

    InvalidateRect(gGameWindow, NULL, TRUE);
}

void MenuItem_OptionsScreen_Back(void)
{
    gCurrentGameState = gPreviousGameState;

    gPreviousGameState = GAMESTATE_OPTIONSSCREEN;

    if (SaveRegistryParameters() != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] SaveRegistryParameters failed!", __FUNCTION__);
    }
}

void DrawOpeningSplashScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0xFF, 0xFF, 0xFF, 0xFF };

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter >= 120)
    {
        if (LocalFrameCounter == 120)
        {
            PlayGameSound(&gSoundSplashScreen);
        }

        if ((LocalFrameCounter >= 180 && LocalFrameCounter <= 210) && (LocalFrameCounter % 15 == 0))
        {
            TextColor.Red   -= 64;

            TextColor.Green -= 64;

            TextColor.Blue  -= 64;
        }

        if (LocalFrameCounter == 225)
        {
            TextColor.Red   = 0;

            TextColor.Green = 0;

            TextColor.Blue  = 0;
        }
        
        if (LocalFrameCounter >= 240)
        {
            gPreviousGameState = gCurrentGameState;

            gCurrentGameState = GAMESTATE_TITLESCREEN;
        }

        BlitStringToBuffer("-Game Studio-", 
            &g6x7Font, 
            &TextColor ,
            (GAME_RES_WIDTH / 2) - ((13 * 6) / 2), 100);

        BlitStringToBuffer("Presents", 
            &g6x7Font, 
            &TextColor, 
            (GAME_RES_WIDTH / 2) - ((8 * 6) / 2), 115);
    }

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;    
}

void DrawTitleScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0x00, 0x00, 0x00, 0x00 };

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        TextColor.Red = 0;

        TextColor.Green = 0;

        TextColor.Blue = 0;

        if (gPlayer.Active == TRUE)
        {
            gMenu_TitleScreen.SelectedItem = 0;
        }
        else
        {
            gMenu_TitleScreen.SelectedItem = 1;
        }
    }   
    
    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter == 10)
    {
        TextColor.Red   = 64;
        
        TextColor.Green = 64;
        
        TextColor.Blue  = 64;        
    }

    if (LocalFrameCounter == 20)
    {
        TextColor.Red   = 128;

        TextColor.Green = 128;

        TextColor.Blue  = 128;
    }

    if (LocalFrameCounter == 30)
    {
        TextColor.Red   = 192;

        TextColor.Green = 192;

        TextColor.Blue  = 192;
    }

    if (LocalFrameCounter == 40)
    {
        TextColor.Red   = 255;

        TextColor.Green = 255;

        TextColor.Blue  = 255;
    }


    //    AARRGGBB ?
    //__stosd(gBackBuffer.Memory, 0xFF0000FF, GAME_DRAWING_AREA_MEMORY_SIZE / sizeof(PIXEL32));

    BlitStringToBuffer(GAME_NAME, &g6x7Font, &TextColor, (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(GAME_NAME) * 6) / 2), 60);

    for (uint8_t MenuItem = 0; MenuItem < gMenu_TitleScreen.ItemCount; MenuItem++)
    {
        if (gMenu_TitleScreen.Items[MenuItem]->Enabled == TRUE)
        {
            BlitStringToBuffer(gMenu_TitleScreen.Items[MenuItem]->Name,
                &g6x7Font,
                &TextColor,
                gMenu_TitleScreen.Items[MenuItem]->x,
                gMenu_TitleScreen.Items[MenuItem]->y);
            
        }
    }

    BlitStringToBuffer("»",
        &g6x7Font,
        &TextColor,
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->x - 6,
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void DrawCharacterNaming(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0x00, 0x00, 0x00, 0x00 };

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        TextColor.Red = 0;

        TextColor.Green = 0;

        TextColor.Blue = 0;
            
        gMenu_CharacterNaming.SelectedItem = 0;
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter == 10)
    {
        TextColor.Red   = 64;

        TextColor.Green = 64;

        TextColor.Blue  = 64;
    }

    if (LocalFrameCounter == 20)
    {
        TextColor.Red   = 128;

        TextColor.Green = 128;

        TextColor.Blue  = 128;
    }

    if (LocalFrameCounter == 30)
    {
        TextColor.Red   = 192;

        TextColor.Green = 192;

        TextColor.Blue  = 192;
    }

    if (LocalFrameCounter == 40)
    {
        TextColor.Red   = 255;

        TextColor.Green = 255;

        TextColor.Blue  = 255;
    }

    BlitStringToBuffer(gMenu_CharacterNaming.Name, &g6x7Font, &TextColor, (GAME_RES_WIDTH / 2) - (((uint16_t)strlen(gMenu_CharacterNaming.Name) * 6) / 2), 16);

    Blit32BppBitmapToBuffer(&gPlayer.Sprite[SUIT_0][FACING_DOWN_0], 125, 64);

    for (uint8_t Letter = 0; Letter < 8; Letter++)
    {
        if (gPlayer.Name[Letter] == '\0')
        {
            BlitStringToBuffer("_", &g6x7Font, &TextColor, 150 + (Letter * 6), 64);
        }
        else
        {
            BlitStringToBuffer(&gPlayer.Name[Letter], &g6x7Font, &TextColor, 150 + (Letter * 6), 64);
        }
    }

    for (uint8_t Counter = 0; Counter < gMenu_CharacterNaming.ItemCount; Counter++)
    {
        BlitStringToBuffer(gMenu_CharacterNaming.Items[Counter]->Name,
            &g6x7Font,
            &TextColor,
            gMenu_CharacterNaming.Items[Counter]->x,
            gMenu_CharacterNaming.Items[Counter]->y);
    }

    BlitStringToBuffer("»",
        &g6x7Font,
        &TextColor,
        gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->x - 6,
        gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;

}

void DrawExitYesNoScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0x00, 0x00, 0x00, 0x00 };

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        TextColor.Red = 0;

        TextColor.Green = 0;

        TextColor.Blue = 0;        
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter == 10)
    {
        TextColor.Red = 64;

        TextColor.Green = 64;

        TextColor.Blue = 64;
    }

    if (LocalFrameCounter == 20)
    {
        TextColor.Red = 128;

        TextColor.Green = 128;

        TextColor.Blue = 128;
    }

    if (LocalFrameCounter == 30)
    {
        TextColor.Red = 192;

        TextColor.Green = 192;

        TextColor.Blue = 192;
    }

    if (LocalFrameCounter == 40)
    {
        TextColor.Red = 255;

        TextColor.Green = 255;

        TextColor.Blue = 255;
    }

    BlitStringToBuffer(gMenu_ExitYesNo.Name, 
        &g6x7Font, 
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_ExitYesNo.Name) * 6) / 2), 
        60);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[0]->Name, 
        &g6x7Font, 
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_ExitYesNo.Items[0]->Name) * 6) / 2), 
        100);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[1]->Name,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_ExitYesNo.Items[1]->Name) * 6) / 2),
        115);

    BlitStringToBuffer("»",
        &g6x7Font,
        &TextColor,
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->x - 6,
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->y);


    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void DrawGamepadUnplugged(void)
{
#define GAMEPADUNPLUGGEDSTRING1 "Gamepad Disconnected!"

#define GAMEPADUNPLUGGEDSTRING2 "Reconnect it, or press escape to continue using the keyboard."

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    BlitStringToBuffer(GAMEPADUNPLUGGEDSTRING1, 
        &g6x7Font, 
        &((PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }), 
        (GAME_RES_WIDTH / 2) - (((uint16_t)strlen(GAMEPADUNPLUGGEDSTRING1) * 6) / 2), 100);
    
    BlitStringToBuffer(GAMEPADUNPLUGGEDSTRING2, 
        &g6x7Font, 
        &((PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }), 
        (GAME_RES_WIDTH / 2) - (((uint16_t)strlen(GAMEPADUNPLUGGEDSTRING2) * 6) / 2), 115);
}

void DrawOptionsScreen(void)
{
    PIXEL32 Grey  = { 0x6F, 0x6F, 0x6F, 0x6F };

    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0x00, 0x00, 0x00, 0x00 };

    char ScreenSizeString[64] = { 0 };

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        TextColor.Red = 0;

        TextColor.Green = 0;

        TextColor.Blue = 0;       
            
        gMenu_OptionsScreen.SelectedItem = 0;        
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter == 10)
    {
        TextColor.Red   = 64;

        TextColor.Green = 64;

        TextColor.Blue  = 64;
    }

    if (LocalFrameCounter == 20)
    {
        TextColor.Red   = 128;

        TextColor.Green = 128;

        TextColor.Blue  = 128;
    }

    if (LocalFrameCounter == 30)
    {
        TextColor.Red   = 192;

        TextColor.Green = 192;

        TextColor.Blue  = 192;
    }

    if (LocalFrameCounter == 40)
    {
        TextColor.Red   = 255;

        TextColor.Green = 255;

        TextColor.Blue  = 255;
    }

    for (uint8_t MenuItem = 0; MenuItem < gMenu_OptionsScreen.ItemCount; MenuItem++)
    {
        if (gMenu_OptionsScreen.Items[MenuItem]->Enabled == TRUE)
        {
            BlitStringToBuffer(gMenu_OptionsScreen.Items[MenuItem]->Name,
                &g6x7Font,
                &TextColor,
                gMenu_OptionsScreen.Items[MenuItem]->x,
                gMenu_OptionsScreen.Items[MenuItem]->y);
        }
    }

    for (uint8_t Volume = 0; Volume < 10; Volume++)
    {
        if (Volume >= (uint8_t)(gSFXVolume * 10))
        {
            if (TextColor.Red == 255)
            {
                BlitStringToBuffer("\xf2", &g6x7Font, &Grey, 224 + (Volume * 6), gMI_OptionsScreen_SFXVolume.y);
            }
        }
        else
        {
            BlitStringToBuffer("\xf2", &g6x7Font, &TextColor, 224 + (Volume * 6), gMI_OptionsScreen_SFXVolume.y);
        }
    }

    for (uint8_t Volume = 0; Volume < 10; Volume++)
    {
        if (Volume >= (uint8_t)(gMusicVolume * 10))
        {
            if (TextColor.Red == 255)
            {
                BlitStringToBuffer("\xf2", &g6x7Font, &Grey, 224 + (Volume * 6), gMI_OptionsScreen_MusicVolume.y);
            }
        }
        else
        {
            BlitStringToBuffer("\xf2", &g6x7Font, &TextColor, 224 + (Volume * 6), gMI_OptionsScreen_MusicVolume.y);
        }
    }

    snprintf(ScreenSizeString, 
        sizeof(ScreenSizeString), 
        "%dx%d", 
        GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor, 
        GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor);

    BlitStringToBuffer(ScreenSizeString, &g6x7Font, &TextColor, 224, gMI_OptionsScreen_ScreenSize.y);    

    BlitStringToBuffer("»",
        &g6x7Font,
        &TextColor,
        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->x - 6,
        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void PPI_OpeningSplashScreen(void)
{
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_TITLESCREEN;
    }
}

void PPI_GamepadUnplugged(void)
{
    if (gGamepadID > -1 || (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown))
    {
        gCurrentGameState = gPreviousGameState;

        gPreviousGameState = GAMESTATE_GAMEPADUNPLUGGED;
    }
}

void PPI_OptionsScreen(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_OptionsScreen.SelectedItem < gMenu_OptionsScreen.ItemCount - 1)
        {
            gMenu_OptionsScreen.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_OptionsScreen.SelectedItem > 0)
        {
            gMenu_OptionsScreen.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }        
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void PPI_TitleScreen(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_TitleScreen.SelectedItem < gMenu_TitleScreen.ItemCount - 1)
        {
            gMenu_TitleScreen.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_TitleScreen.SelectedItem > 0)
        {
            if (gMenu_TitleScreen.SelectedItem == 1) // Don't move to "Resume" if there is no game currently in progress.
            {
                if (gPlayer.Active)
                {
                    gMenu_TitleScreen.SelectedItem--;

                    PlayGameSound(&gSoundMenuNavigate);
                }
            }
            else
            {
                gMenu_TitleScreen.SelectedItem--;

                PlayGameSound(&gSoundMenuNavigate);
            }
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void PPI_CharacterNaming(void)
{
    // Character Naming Menu

    // A B C D E F G H I J K L M 
    // N O P Q R S T U V W X Y Z
    // a b c d e f g h i j k l m
    // n o p q r s t u v w x y z
    // Back                   OK


#define BACK_BUTTON 52

#define OK_BUTTON   53

#define CAPITAL_M   12

#define LOWERCASE_N 37

#define LOWERCASE_Z 51

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem > CAPITAL_M)
        {
            gMenu_CharacterNaming.SelectedItem -= 13;            
        }
        else
        {
            gMenu_CharacterNaming.SelectedItem = BACK_BUTTON;            
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem < 39)
        {
            gMenu_CharacterNaming.SelectedItem += 13;            
        }
        else if (gMenu_CharacterNaming.SelectedItem >= LOWERCASE_N)
        {
            gMenu_CharacterNaming.SelectedItem = BACK_BUTTON;
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem == 0 ||
            gMenu_CharacterNaming.SelectedItem == 13 ||
            gMenu_CharacterNaming.SelectedItem == 26 ||
            gMenu_CharacterNaming.SelectedItem == 39)
        {
            gMenu_CharacterNaming.SelectedItem += 12;            
        }
        else
        {
            gMenu_CharacterNaming.SelectedItem--;
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.RightKeyIsDown && !gGameInput.RightKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem == 12 ||
            gMenu_CharacterNaming.SelectedItem == 25 ||
            gMenu_CharacterNaming.SelectedItem == 38 ||
            gMenu_CharacterNaming.SelectedItem == LOWERCASE_Z)
        {
            gMenu_CharacterNaming.SelectedItem -= 12;            
        }
        else
        {
            if (gMenu_CharacterNaming.SelectedItem < OK_BUTTON)
            {
                gMenu_CharacterNaming.SelectedItem++;
            }
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->Action();        
    }

    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        MenuItem_CharacterNaming_Back();
    }
}

void PPI_Overworld(void)
{
    if (!gPlayer.MovementRemaining)
    {
        if (gGameInput.DownKeyIsDown)
        {
            if (gPlayer.ScreenPosY < GAME_RES_HEIGHT - 16)
            {
                gPlayer.Direction = DOWN;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.LeftKeyIsDown)
        {
            if (gPlayer.ScreenPosX > 0)
            {
                gPlayer.Direction = LEFT;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.RightKeyIsDown)
        {
            if (gPlayer.ScreenPosX < GAME_RES_WIDTH - 16)
            {
                gPlayer.Direction = RIGHT;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.UpKeyIsDown)
        {
            if (gPlayer.ScreenPosY > 0)
            {
                gPlayer.Direction = UP;

                gPlayer.MovementRemaining = 16;
            }
        }
    }
    else
    {
        gPlayer.MovementRemaining--;

        if (gPlayer.Direction == DOWN)
        {
            gPlayer.ScreenPosY++;
        }
        else if (gPlayer.Direction == LEFT)
        {
            gPlayer.ScreenPosX--;
        }
        else if (gPlayer.Direction == RIGHT)
        {
            gPlayer.ScreenPosX++;
        }
        else if (gPlayer.Direction == UP)
        {
            gPlayer.ScreenPosY--;
        }

        switch (gPlayer.MovementRemaining)
        {
            case 16:
            {
                gPlayer.SpriteIndex = 0;

                break;
            }
            case 12:
            {
                gPlayer.SpriteIndex = 1;

                break;
            }
            case 8:
            {
                gPlayer.SpriteIndex = 0;

                break;
            }
            case 4:
            {
                gPlayer.SpriteIndex = 2;

                break;
            }
            case 0:
            {
                gPlayer.SpriteIndex = 0;

                break;
            }
            default:
            {

            }
        }
    }
}

void PPI_ExitYesNo(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem < gMenu_ExitYesNo.ItemCount - 1)
        {
            gMenu_ExitYesNo.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem > 0)
        {
            gMenu_ExitYesNo.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

HRESULT InitializeSoundEngine(void)
{
    HRESULT Result = S_OK;

    WAVEFORMATEX SfxWaveFormat = { 0 };

    WAVEFORMATEX MusicWaveFormat = { 0 };


    Result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (Result != S_OK)
    {
        LogMessageA(LL_ERROR, "[%s] CoInitializeEx failed with 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    Result = XAudio2Create(&gXAudio, 0, XAUDIO2_ANY_PROCESSOR);

    if (FAILED(Result))
    {
        LogMessageA(LL_ERROR, "[%s] XAudio2Create failed with 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    Result = gXAudio->lpVtbl->CreateMasteringVoice(gXAudio, &gXAudioMasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL, 0);

    if (FAILED(Result))
    {
        LogMessageA(LL_ERROR, "[%s] CreateMasteringVoice failed with 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    SfxWaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    SfxWaveFormat.nChannels = 1; // Mono

    SfxWaveFormat.nSamplesPerSec = 44100;

    SfxWaveFormat.nAvgBytesPerSec = SfxWaveFormat.nSamplesPerSec * SfxWaveFormat.nChannels * 2;

    SfxWaveFormat.nBlockAlign = SfxWaveFormat.nChannels * 2;

    SfxWaveFormat.wBitsPerSample = 16;

    SfxWaveFormat.cbSize = 0x6164;

    for (uint8_t Counter = 0; Counter < NUMBER_OF_SFX_SOURCE_VOICES; Counter++)
    {
        Result = gXAudio->lpVtbl->CreateSourceVoice(gXAudio, &gXAudioSFXSourceVoice[Counter], &SfxWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);

        if (Result != S_OK)
        {
            LogMessageA(LL_ERROR, "[%s] CreateSourceVoice failed with 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        gXAudioSFXSourceVoice[Counter]->lpVtbl->SetVolume(gXAudioSFXSourceVoice[Counter], gSFXVolume, XAUDIO2_COMMIT_NOW);
    }

    MusicWaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    MusicWaveFormat.nChannels = 2; // Stereo

    MusicWaveFormat.nSamplesPerSec = 44100;

    MusicWaveFormat.nAvgBytesPerSec = MusicWaveFormat.nSamplesPerSec * MusicWaveFormat.nChannels * 2;

    MusicWaveFormat.nBlockAlign = MusicWaveFormat.nChannels * 2;

    MusicWaveFormat.wBitsPerSample = 16;

    MusicWaveFormat.cbSize = 0;

    Result = gXAudio->lpVtbl->CreateSourceVoice(gXAudio, &gXAudioMusicSourceVoice, &MusicWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);

    if (Result != S_OK)
    {
        LogMessageA(LL_ERROR, "[%s] CreateSourceVoice failed with 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    gXAudioMusicSourceVoice->lpVtbl->SetVolume(gXAudioMusicSourceVoice, gMusicVolume, XAUDIO2_COMMIT_NOW);
        

Exit:

    return(Result);
}

DWORD LoadWavFromFile(_In_ char* FileName, _Inout_ GAMESOUND* GameSound)
{
    DWORD Error = ERROR_SUCCESS;

    DWORD NumberOfBytesRead = 0;

    DWORD RIFF = 0;

    uint16_t DataChunkOffset = 0;

    DWORD DataChunkSearcher = 0;

    BOOL DataChunkFound = FALSE;

    DWORD DataChunkSize = 0;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;    

    void* AudioData = NULL;


    if ((FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] CreateFileA failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if (ReadFile(FileHandle, &RIFF, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if (RIFF != 0x46464952) // "RIFF" backwards
    {
        Error = ERROR_FILE_INVALID;

        LogMessageA(LL_ERROR, "[%s] First four bytes of this file are not 'RIFF'!", __FUNCTION__, Error);

        goto Exit;
    }

    if (SetFilePointer(FileHandle, 20, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] SetFilePointer failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if (ReadFile(FileHandle, &GameSound->WaveFormat, sizeof(WAVEFORMATEX), &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if (GameSound->WaveFormat.nBlockAlign != ((GameSound->WaveFormat.nChannels * GameSound->WaveFormat.wBitsPerSample) / 8) ||
        (GameSound->WaveFormat.wFormatTag != WAVE_FORMAT_PCM) ||
        (GameSound->WaveFormat.wBitsPerSample != 16))
    {
        Error = ERROR_DATATYPE_MISMATCH;

        LogMessageA(LL_ERROR, "[%s] This wav file did not meet the format requirements! Only PCM format, 44.1KHz, 16 bits per sample wav files are supported. 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    while (DataChunkFound == FALSE)
    {
        if (SetFilePointer(FileHandle, DataChunkOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        {
            Error = GetLastError();

            LogMessageA(LL_ERROR, "[%s] SetFilePointer failed with 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        if (ReadFile(FileHandle, &DataChunkSearcher, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
        {
            Error = GetLastError();

            LogMessageA(LL_ERROR, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        if (DataChunkSearcher == 0x61746164) // 'data', backwards
        {
            DataChunkFound = TRUE;

            break;
        }
        else
        {
            DataChunkOffset += 4;
        }

        if (DataChunkOffset > 256)
        {
            Error = ERROR_DATATYPE_MISMATCH;

            LogMessageA(LL_ERROR, "[%s] Data chunk not found within first 256 bytes of this file! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }
    }

    if (SetFilePointer(FileHandle, DataChunkOffset + 4, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] SetFilePointer failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if (ReadFile(FileHandle, &DataChunkSize, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    AudioData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DataChunkSize);

    if (AudioData == NULL)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;

        LogMessageA(LL_ERROR, "[%s] HeapAlloc failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = DataChunkSize;

    if (SetFilePointer(FileHandle, DataChunkOffset + 8, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] SetFilePointer failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if (ReadFile(FileHandle, AudioData, DataChunkSize, &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        LogMessageA(LL_ERROR, "[%s] ReadFile failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    GameSound->Buffer.pAudioData = AudioData;

Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Successfully loaded %s.", __FUNCTION__, FileName);
    }
    else
    {
        LogMessageA(LL_ERROR, "[%s] Failed to load %s! Error: 0x%08lx!", __FUNCTION__, FileName, Error);
    }

    if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
    {
        CloseHandle(FileHandle);
    }

    return(Error);
}

void PlayGameSound(_In_ GAMESOUND* GameSound)
{
    gXAudioSFXSourceVoice[gSFXSourceVoiceSelector]->lpVtbl->SubmitSourceBuffer(gXAudioSFXSourceVoice[gSFXSourceVoiceSelector], &GameSound->Buffer, NULL);

    gXAudioSFXSourceVoice[gSFXSourceVoiceSelector]->lpVtbl->Start(gXAudioSFXSourceVoice[gSFXSourceVoiceSelector], 0, XAUDIO2_COMMIT_NOW);

    gSFXSourceVoiceSelector++;

    if (gSFXSourceVoiceSelector > (NUMBER_OF_SFX_SOURCE_VOICES - 1))
    {
        gSFXSourceVoiceSelector = 0;
    }
}

