// Filename: Main.c
// Contains WinMain, which is the entry point of the entire game, as well as code for functions
// that are common across multiple game states or are considered fundamental infrastructure.
//
// Project Codename: GameB
// TODO: Come up with a better name later.
// 2020 Joseph Ryan Ries <ryanries09@gmail.com>
// My YouTube series where we program an entire video game from scratch in C.
// Watch it on YouTube:    https://www.youtube.com/watch?v=3zFFrBSdBvA
// Follow along on GitHub: https://github.com/ryanries/GameB
// Find me on Twitter @JosephRyanRies 
// # License
// ----------
// The source code in this project is licensed under the MIT license.
// The media assets such as artwork, custom fonts, music and sound effects are licensed under a separate license.
// A copy of that license can be found in the 'Assets' directory.
// stb_vorbis by Sean Barrett is public domain and a copy of its license can be found in the stb_vorbis.c file.
// miniz by Rich Geldreich <richgel99@gmail.com> is public domain (or possibly MIT licensed) and a copy of its license can be found in the miniz.c file.

// --- TODO ---
// Can't hit F1 for debug statistics until essential assets event is set
// Create a windowing system
// enhance Blit32BppBitmap function so that it can alter the color and brightness of bitmaps at run time
// maybe a new MAP data structure for map GAMEBITMAP plus TILEMAP together? (plus default GAMESOUND too?)
// talk about the scope of #define precompiler directives
// disable keyboard input during gamestate fade-in
// shadow effect for text?
// gradient effect for text?
// should we enhance BlitStringToBuffer to support varargs?
// overworld... tile maps...
// Optimization: String caching?
// Optimization: Should we defer the loading of some assets or just load them all as soon as the game is launched?
// Add logging to InitializeHero
//
// Add movement speed to player
//
// use enums instead of #defines ?
//
// LogMessageA option to spawn msgbox ? And assert ?
//
// Blit32bppbitmap draw off the edge of the screen ?
//
// Screen size code for monitors that are not 16 : 9
//
// Menus
// 
// Game states (Maybe each state should be a struct that includes an array of valid gamestates that it may transition to?)
//
// Tile maps

// Contains global declarations shared among multiple files.
#include "Main.h"

// Contains declarations that are specific to the Opening Splash Screen game state.
#include "OpeningSplashScreen.h"

// Contains declarations that are specific to the Title Screen game state.
#include "TitleScreen.h"

// Contains declarations that are specific to the Options Screen game state.
#include "OptionsScreen.h"

// Contains declarations that are specific to the Exit Yes/No Screen game state.
#include "ExitYesNoScreen.h"

// Contains declarations that are specific to the Gamepad Unplugged Screen game state.
#include "GamepadUnplugged.h"

// Contains declarations that are specific to the Character Naming Screen game state.
#include "CharacterNamingScreen.h"

// Contains declarations that are specific to the Overworld game state.
#include "Overworld.h"

// Miniz zip file [de]compression by Rich Geldreich <richgel99@gmail.com>, but we've modified some of 
// the constants so that the exact file format is only readable by our game and not common
// tools such as 7zip, WinRAR, etc.
#include "miniz.h"      

// This critical section is used to synchronize LogMessageA between multiple threads.
CRITICAL_SECTION gLogCritSec;

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

// If the game window does not have focus, then do not process player input.
BOOL gWindowHasFocus;

// Any values that are read from and stored in Windows registry go in this struct.
REGISTRYPARAMS gRegistryParams;

// Holds the state of the Xbox360 gamepad, e.g., is the X button currently pushed or not?
XINPUT_STATE gGamepadState;

// The COM interface for XAudio2, which is a Microsoft audio library that we 
// use for playing sounds and music. XAudio2 uses the concept of source voices and
// mastering voices. We submit audio data to the source voice, and the source
// voice sends that to the mastering voice, which mixes everything together and sends the final audio 
// data to the playback device.
// We only use a single mastering voice, but we use one source voice for background music and 
// we use 4 source voices for sound effects. By using 4 source voices for sound effects, we can
// play up to 4 SFX simultaneously. Otherwise, submitting multiple buffers to the same source voice
// before the previous sound has finished playing will simply queue up the sounds to play one after
// the other.
IXAudio2* gXAudio;

IXAudio2MasteringVoice* gXAudioMasteringVoice;

// This is an iterator used to rapidly cycle through source voices 0-3 so we can play
// up to 4 sound effects simultaneously.
uint8_t gSFXSourceVoiceSelector;





// The game's entry point.
int __stdcall WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ PSTR CommandLine, _In_ INT CmdShow)
{
    UNREFERENCED_PARAMETER(Instance);

	UNREFERENCED_PARAMETER(PreviousInstance);

	UNREFERENCED_PARAMETER(CommandLine);

	UNREFERENCED_PARAMETER(CmdShow);
    
    // Handles Windows' Window messages to our game's window.
    MSG Message = { 0 };

    // The time in microseconds that each frame starts.
    int64_t FrameStart = 0;

    // The time in microseconds that each frame ends. 
    // This variable is reused to measure both "raw" and "cooked" frame times.
    int64_t FrameEnd = 0;

    // The elapsed time in microseconds that each frame ends. 
    // This variable is reused to measure both "raw" and "cooked" frame times. 
    int64_t ElapsedMicroseconds = 0;

    // We accumulate the amount of time taken to render a "raw" frame and then
    // calculate the average every x frames, i.e., we divide it by x number of frames rendered.
    int64_t ElapsedMicrosecondsAccumulatorRaw = 0;

    // We accumulate the amount of time taken to render a "cooked" frame and then
    // calculate the average every x frames, i.e., we divide it by x number of frames rendered.
    // This should hopefully be a steady 60fps. A "cooked" frame is a frame after we have 
    // waited/slept for a calculated amount of time before starting the next frame.
    int64_t ElapsedMicrosecondsAccumulatorCooked = 0;    

    // The time the game started. This is used to calculate CPU usage percentage.
    FILETIME ProcessCreationTime = { 0 };

    // This is needed by the GetProcessTimes function, but we don't use it.
    FILETIME ProcessExitTime = { 0 };

    // These four variables are used to calculate the average CPU usage.
    int64_t CurrentUserCPUTime = 0;

    int64_t CurrentKernelCPUTime = 0;

    int64_t PreviousUserCPUTime = 0;

    int64_t PreviousKernelCPUTime = 0;
    

    // This critical section is used to synchronize access to the log file vis a vis
    // LogMessageA when used by multiple threads simultaneously. Documentation says
    // this function never fails, therefore checking its return code is not important.
    // But the critical section does need to be initialized before attempting to log anything.
#pragma warning(suppress: 6031)
    InitializeCriticalSectionAndSpinCount(&gLogCritSec, 0x400);

    // This event gets signalled/set after the most essential assets have been loaded.
    // "Essential" means the assets required to render the splash screen.
    if ((gEssentialAssetsLoadedEvent = CreateEventA(NULL, TRUE, FALSE, "gEssentialAssetsLoadedEvent")) == NULL)
    {
        MessageBoxA(NULL, "CreateEvent failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    // LoadRegistryParameters should be the first thing that runs, and no logging
    // should be attempted before this function runs, because the user-configurable
    // logging verbosity level is defined in the registry and is thus retrieved by this function.
    if (LoadRegistryParameters() != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadRegistryParameters failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] %s %s is starting.", __FUNCTION__, GAME_NAME, GAME_VER);

    // This function uses a global mutex to determine whether another instance of the same
    // process is already running. This is not hack-proof but it does prevent accidents.
    if (GameIsAlreadyRunning() == TRUE)
    {
        LogMessageA(LL_ERROR, "[%s] Another instance of this program is already running!", __FUNCTION__);

        MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    // --- TODO: 
    // Wrap these things into some function like "InitializeGlobals()"

    //gCurrentGameState = GAMESTATE_OVERWORLD;

    gGameIsRunning = TRUE;

    gGamepadID = -1;

    gPassableTiles[0] = TILE_GRASS_01;



    // C99 syntax?
    gOverworldArea = (RECT){ .left = 0, .top = 0, .right = 3840, .bottom = 2400 };

    // --- END TODO


    // We need the undocumented Windows API function NtQueryTimerResolution to get the resolution of the global system timer.
    // A higher resolution timer will show a lower number, because if your clock can tick every e.g. 0.5ms, that is a higher 
    // resolution than a timer that can only tick every 1.0ms.
    if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryTimerResolution")) == NULL)
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

    // Initialize the variable that is used to calculate average CPU usage of this process.
    GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.PreviousSystemTime);

    // The timeBeginPeriod function controls a global system-wide timer. So
    // increasing the clock resolution here will affect all processes running on this
    // entire system. It will increase context switching, the rate at which timers
    // fire across the entire system, etc. Due to this, Microsoft generally discourages 
    // the use of timeBeginPeriod completely. However, we need it, because without 
    // 1ms timer resolution, we simply cannot maintain a reliable 60 frames per second.
    // Also, we don't need to worry about calling timeEndPeriod, because Windows will
    // automatically cancel our requested timer resolution once this process exits.
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

    // Increase process and thread priority to minimize the chances of another thread on the system
    // preempting us when we need to run and causing a stutter in our frame rate. (Though it can still happen.)
    if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS) == 0)
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

        MessageBoxA(NULL, "Failed to create game window!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if ((GetFileAttributesA(ASSET_FILE) == INVALID_FILE_ATTRIBUTES) || 
        (GetFileAttributesA(ASSET_FILE) & FILE_ATTRIBUTE_DIRECTORY))
    {
        LogMessageA(LL_ERROR, "[%s] The asset file %s was not found! It must reside in the same directory as the game executable.", __FUNCTION__, ASSET_FILE);

        MessageBoxA(NULL, "The asset file was not found! It must reside in the same directory as the game executable.", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if ((gAssetLoadingThreadHandle = CreateThread(NULL, 0, AssetLoadingThreadProc, NULL, 0, NULL)) == NULL)
    {
        MessageBoxA(NULL, "CreateThread failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    } 

    if (InitializeSoundEngine() != S_OK)
    {
        MessageBoxA(NULL, "InitializeSoundEngine failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }


    
    QueryPerformanceFrequency((LARGE_INTEGER*)&gPerformanceData.PerfFrequency);


    // This is our global backbuffer onto which we will draw all of our graphics.
    // This buffer gets drawn and blitted onto the screen 60 times per second.
    // We don't have to worry about freeing the memory allocated for this buffer
    // because it is intended to last throughout the entire lifetime of the process.
    // Windows will automatically deallocate this memory when the game exits.
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

#ifdef _DEBUG

    // If this is a debug build, initialize the memory for the backbuffer to 0xCC
    // Incidentally, the byte 0xCC is also int 3, which is the CPU instruction that causes
    // a debug break.

    memset(gBackBuffer.Memory, 0xCC, GAME_DRAWING_AREA_MEMORY_SIZE);

#endif

    if (InitializeHero() != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to initialize hero!", __FUNCTION__);

        MessageBoxA(NULL, "Failed to initialize hero!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }


    // This is the main game loop. Setting gGameIsRunning to FALSE at any point will cause
    // the game to exit immediately. The loop has two important functions: ProcessPlayerInput
    // and RenderFrameGraphics. The loop will execute these two duties as quickly as possible,
    // but will then sleep for a few milliseconds using a precise timing mechanism in order 
    // to achieve a smooth 60 frames per second. We also calculate some performance statistics
    // every 2 seconds or 120 frames.    

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
            // Sleep(1) is only anywhere near 1 millisecond if we have previously set the global
            // system timer resolution to 1ms or below using timeBeginPeriod.

            if (ElapsedMicroseconds < (TARGET_MICROSECONDS_PER_FRAME * 0.75f))
            {                
                Sleep(1);
            }
        }

        ElapsedMicrosecondsAccumulatorCooked += ElapsedMicroseconds;

        if ((gPerformanceData.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES) == 0)
        {
            GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.CurrentSystemTime);

            GetProcessTimes(GetCurrentProcess(),
                &ProcessCreationTime, 
                &ProcessExitTime, 
                (FILETIME*)&CurrentKernelCPUTime,
                (FILETIME*)&CurrentUserCPUTime);

            gPerformanceData.CPUPercent = (double)(CurrentKernelCPUTime - PreviousKernelCPUTime) + (CurrentUserCPUTime - PreviousUserCPUTime);

            gPerformanceData.CPUPercent /= (gPerformanceData.CurrentSystemTime - gPerformanceData.PreviousSystemTime);

            gPerformanceData.CPUPercent /= gPerformanceData.SystemInfo.dwNumberOfProcessors;

            gPerformanceData.CPUPercent *= 100;

            GetProcessHandleCount(GetCurrentProcess(), &gPerformanceData.HandleCount);

            K32GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&gPerformanceData.MemInfo, sizeof(gPerformanceData.MemInfo));

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

    LogMessageA(LL_INFO, "[%s] Game is exiting.\r\n", __FUNCTION__);

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
//#ifdef _DEBUG
//    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
//#else
    WindowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
//#endif

    WindowClass.lpszMenuName = NULL;

    WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";   

    // This is set in the application manifest, as recommended by MSDN.
    // Not recommended to set this via code so that is why this line is commented out.
    // SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();

        LogMessageA(LL_ERROR, "[%s] RegisterClassExA failed! Error 0x%08lx!", __FUNCTION__, Result);        

        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, 
        WindowClass.lpszClassName, 
        GAME_NAME, 
        WS_VISIBLE, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        640, 
        480, 
        NULL, 
        NULL, 
        GetModuleHandleA(NULL), 
        NULL);

    if (gGameWindow == NULL)
    {
        Result = GetLastError();

        LogMessageA(LL_ERROR, "[%s] CreateWindowExA failed! Error 0x%08lx!", __FUNCTION__, Result);        

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

DWORD InitializeHero(void)
{
    gPlayer.ScreenPos.x = 192;

    gPlayer.ScreenPos.y = 64;

    gPlayer.WorldPos.x = 192;

    gPlayer.WorldPos.y = 64;

    gPlayer.CurrentArmor = SUIT_0;

    gPlayer.Direction = DOWN;

    //gCamera.x = 3456;

    //gCamera.y = 0;

    return(0);    
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
            DrawOverworld();

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

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t x, _In_ int16_t y)
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
            // Do not attempt to draw pixels that are outside of the viewable screen.
            // You will crash if you attempt to draw a pixel that is outside of the bounds of
            // gBackBuffer.Memory. Or you will cause a "wrap-around" effect if you try to draw
            // beyond GAME_RES_WIDTH.
            // TODO: OPTIMIZE THIS???
            if ((x < 1) || (x > GAME_RES_WIDTH - GameBitmap->BitmapInfo.bmiHeader.biWidth) || 
                (y < 1) || (y > GAME_RES_HEIGHT - GameBitmap->BitmapInfo.bmiHeader.biHeight))
            {
                if (x < 1)
                {
                    if (XPixel < -x)
                    {
                        break;
                    }
                }
                else if (x > GAME_RES_WIDTH - GameBitmap->BitmapInfo.bmiHeader.biWidth)
                {
                    if (XPixel > GAME_RES_WIDTH - x - 1)
                    {
                        break;
                    }
                }

                if (y < 1)
                {
                    if (YPixel < -y)
                    {
                        break;
                    }
                }
                else if (y > GAME_RES_HEIGHT - GameBitmap->BitmapInfo.bmiHeader.biHeight)
                {
                    if (YPixel > GAME_RES_HEIGHT - y - 1)
                    {
                        break;
                    }
                }
            }            

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

// Draws a subsection of a background across the entire screen.
// The background may be an entire overworld map which may be much larger than the screen.
// Uses gCamera to control which part of the background gets drawn to the screen.
// The camera is panned around based on the character's movement. E.g., when the player
// walks toward the edge of the screen, the camera gets pushed in that direction.
void BlitBackgroundToBuffer(_In_ GAMEBITMAP* GameBitmap)
{
    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH);

    int32_t StartingBitmapPixel = ((GameBitmap->BitmapInfo.bmiHeader.biWidth * GameBitmap->BitmapInfo.bmiHeader.biHeight) - \
        GameBitmap->BitmapInfo.bmiHeader.biWidth) + gCamera.x - (GameBitmap->BitmapInfo.bmiHeader.biWidth * gCamera.y);

    int32_t MemoryOffset = 0;

    int32_t BitmapOffset = 0;
#ifdef AVX
    __m256i BitmapOctoPixel;

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel += 8)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

            BitmapOctoPixel = _mm256_loadu_si256((const __m256i*)((PIXEL32*)GameBitmap->Memory + BitmapOffset));

            _mm256_store_si256((__m256i*)((PIXEL32*)gBackBuffer.Memory + MemoryOffset), BitmapOctoPixel);
        }
    }
#elif defined SSE2    
    __m128i BitmapQuadPixel;

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel += 4)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

            BitmapQuadPixel = _mm_load_si128((const __m128i*)((PIXEL32*)GameBitmap->Memory + BitmapOffset));

            _mm_store_si128((__m128i*)((PIXEL32*)gBackBuffer.Memory + MemoryOffset), BitmapQuadPixel);
        }
    }


#else
    PIXEL32 BitmapPixel = { 0 };

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel++)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
        }
    }
#endif
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

    EnterCriticalSection(&gLogCritSec);

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
    
    LeaveCriticalSection(&gLogCritSec);
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

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "ScreenXY:%hu,%hu", gPlayer.ScreenPos.x, gPlayer.ScreenPos.y);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 72);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "WorldXY: %hu,%hu", gPlayer.WorldPos.x, gPlayer.WorldPos.y);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 80);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CameraXY:%hu,%hu", gCamera.x, gCamera.y);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, &White, 0, 88);
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



// A compressed *.wav file has been extracted from the assets file and now resides
// in heap memory. This function parses that memory and populates a GAMESOUND data structure.
// This function should probably only be called from LoadAssetFromArchive.
DWORD LoadWavFromMemory(_In_ void* Buffer, _Inout_ GAMESOUND* GameSound)
{
    DWORD Error = ERROR_SUCCESS;    

    DWORD RIFF = 0;

    uint16_t DataChunkOffset = 0;

    DWORD DataChunkSearcher = 0;

    BOOL DataChunkFound = FALSE;

    DWORD DataChunkSize = 0;

    memcpy(&RIFF, Buffer, sizeof(DWORD));

    if (RIFF != 0x46464952) // "RIFF" backwards
    {
        Error = ERROR_FILE_INVALID;

        LogMessageA(LL_ERROR, "[%s] First four bytes of memory buffer are not 'RIFF'!", __FUNCTION__, Error);

        goto Exit;
    }

    // 20 bytes into a wav file, there is a WAVEFORMATEX data structure.
    memcpy(&GameSound->WaveFormat, (BYTE*)Buffer + 20, sizeof(WAVEFORMATEX));

    if (GameSound->WaveFormat.nBlockAlign != ((GameSound->WaveFormat.nChannels * GameSound->WaveFormat.wBitsPerSample) / 8) ||
        (GameSound->WaveFormat.wFormatTag != WAVE_FORMAT_PCM) ||
        (GameSound->WaveFormat.wBitsPerSample != 16))
    {
        Error = ERROR_DATATYPE_MISMATCH;

        LogMessageA(LL_ERROR, "[%s] This wav data in the memory buffer did not meet the format requirements! Only PCM format, 44.1KHz, 16 bits per sample wav files are supported. 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    // We search for the data chunk, which is an indeterminite number of bytes into the file/buffer.
    while (DataChunkFound == FALSE)
    {
        memcpy(&DataChunkSearcher, (BYTE*)Buffer + DataChunkOffset, sizeof(DWORD));

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

            LogMessageA(LL_ERROR, "[%s] Data chunk not found within first 256 bytes of the memory buffer! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }
    }

    memcpy(&DataChunkSize, (BYTE*)Buffer + DataChunkOffset + 4, sizeof(DWORD));

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = DataChunkSize;

    GameSound->Buffer.pAudioData = (BYTE*)Buffer + DataChunkOffset + 8;

Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Successfully loaded wav from memory.", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_ERROR, "[%s] Failed to load wav from memory! Error: 0x%08lx!", __FUNCTION__, Error);
    }

    return(Error);
}

DWORD LoadOggFromMemory(_In_ void* Buffer, _In_ uint32_t BufferSize, _Inout_ GAMESOUND* GameSound)
{
    DWORD Error = ERROR_SUCCESS;    

    int SamplesDecoded = 0;

    int Channels = 0;

    int SampleRate = 0;

    short* DecodedAudio = NULL;    

    SamplesDecoded = stb_vorbis_decode_memory(Buffer, (int)BufferSize, &Channels, &SampleRate, &DecodedAudio);

    if (SamplesDecoded < 1)
    {
        Error = ERROR_BAD_COMPRESSION_BUFFER;

        LogMessageA(LL_ERROR, "[%s] stb_vorbis_decode_memory failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    GameSound->WaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    GameSound->WaveFormat.nChannels = (WORD)Channels;

    GameSound->WaveFormat.nSamplesPerSec = SampleRate;

    GameSound->WaveFormat.nAvgBytesPerSec = GameSound->WaveFormat.nSamplesPerSec * GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.nBlockAlign = GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.wBitsPerSample = 16;

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = SamplesDecoded * GameSound->WaveFormat.nChannels * 2;

    GameSound->Buffer.pAudioData = (const BYTE*)DecodedAudio;

Exit:

    return(Error);
}

DWORD LoadTilemapFromMemory(_In_ void* Buffer, _In_ uint32_t BufferSize, _Inout_ TILEMAP* TileMap)
{
    DWORD Error = ERROR_SUCCESS;

    DWORD BytesRead = 0;

    char* Cursor = NULL;

    char TempBuffer[16] = { 0 };

    uint16_t Rows = 0;

    uint16_t Columns = 0;


    if (BufferSize < 300)
    {
        Error = ERROR_FILE_INVALID;

        LogMessageA(LL_ERROR, "[%s] Buffer is too small to be a valid tile map! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Cursor = strstr(Buffer, "width=")) == NULL)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Could not locate the width attribute! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    BytesRead = 0;

    for (;;)
    {
        if (BytesRead > 8)
        {
            // We should have found the opening quotation mark by now.

            Error = ERROR_INVALID_DATA;

            LogMessageA(LL_ERROR, "[%s] Could not locate the opening quotation mark before the width attribute! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        BytesRead++;
    }

    BytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Width = (uint16_t)atoi(TempBuffer);

    if (TileMap->Width == 0)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Width attribute was 0! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(Buffer, "height=")) == NULL)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Could not locate the height attribute! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    BytesRead = 0;

    for (;;)
    {
        if (BytesRead > 8)
        {
            // We should have found the opening quotation mark by now.

            Error = ERROR_INVALID_DATA;

            LogMessageA(LL_ERROR, "[%s] Could not locate the opening quotation mark before the height attribute! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        BytesRead++;
    }

    BytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Height = (uint16_t)atoi(TempBuffer);

    if (TileMap->Height == 0)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Height attribute was 0! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] TileMap dimensions: %dx%d.", __FUNCTION__, TileMap->Width, TileMap->Height);

    Rows = TileMap->Height;

    Columns = TileMap->Width;

    TileMap->Map = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Rows * sizeof(void*));

    if (TileMap->Map == NULL)
    {
        Error = ERROR_OUTOFMEMORY;

        LogMessageA(LL_ERROR, "[%s] HeapAlloc failed! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    for (uint16_t Counter = 0; Counter < TileMap->Height; Counter++)
    {
        TileMap->Map[Counter] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Columns * sizeof(void*));

        if (TileMap->Map[Counter] == NULL)
        {
            Error = ERROR_OUTOFMEMORY;

            LogMessageA(LL_ERROR, "[%s] HeapAlloc failed! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }
    }

    BytesRead = 0;

    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(Buffer, ",")) == NULL)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Could not find a comma character! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    while (*Cursor != '\r' && *Cursor != '\n')
    {
        if (BytesRead > 4)
        {
            Error = ERROR_INVALID_DATA;

            LogMessageA(LL_ERROR, "[%s] Could not find a new line character at the beginning of the tile map data! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        BytesRead++;

        Cursor--;
    }

    Cursor++;

    for (uint16_t Row = 0; Row < Rows; Row++)
    {
        for (uint16_t Column = 0; Column < Columns; Column++)
        {
            memset(TempBuffer, 0, sizeof(TempBuffer));

        Skip:

            if (*Cursor == '\r' || *Cursor == '\n')
            {
                Cursor++;

                goto Skip;
            }

            for (uint8_t Counter = 0; Counter < 8; Counter++)
            {
                if (*Cursor == ',' || *Cursor == '<')
                {
                    if (((TileMap->Map[Row][Column]) = (uint8_t)atoi(TempBuffer)) == 0)
                    {
                        Error = ERROR_INVALID_DATA;

                        LogMessageA(LL_ERROR, "[%s] atoi failed while converting tile map data! (The tilemap cannot contain any tiles with the value 0, because atoi cannot differentiate between 0 and failure.) 0x%08lx!", __FUNCTION__, Error);

                        goto Exit;
                    }

                    Cursor++;

                    break;
                }

                TempBuffer[Counter] = *Cursor;

                Cursor++;
            }
        }
    }


Exit:

    // TODO: Can this memory be freed? I don't think we need this XML document in memory anymore...
    //if (Buffer)
    //{
    //    HeapFree(GetProcessHeap(), 0, Buffer);
    //}

    return(Error);
}

DWORD Load32BppBitmapFromMemory(_In_ void* Buffer, _Inout_ GAMEBITMAP* GameBitmap)
{
    DWORD Error = ERROR_SUCCESS;    

    WORD BitmapHeader = 0;

    DWORD PixelDataOffset = 0;

    //DWORD NumberOfBytesRead = 2;

    memcpy(&BitmapHeader, Buffer, sizeof(WORD));

    if (BitmapHeader != 0x4d42) // "BM" backwards
    {
        Error = ERROR_INVALID_DATA;

        goto Exit;
    }

    memcpy(&PixelDataOffset, (BYTE*)Buffer + 0xA, sizeof(DWORD));

    memcpy(&GameBitmap->BitmapInfo.bmiHeader, (BYTE*)Buffer + 0xE, sizeof(BITMAPINFOHEADER));

    GameBitmap->Memory = (BYTE*)Buffer + PixelDataOffset;

Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Loading successful.", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_ERROR, "[%s] Loading failed! Error 0x%08lx!", __FUNCTION__, Error);
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

void PlayGameMusic(_In_ GAMESOUND* GameSound)
{
    gXAudioMusicSourceVoice->lpVtbl->Stop(gXAudioMusicSourceVoice, 0, 0);

    gXAudioMusicSourceVoice->lpVtbl->FlushSourceBuffers(gXAudioMusicSourceVoice);

    GameSound->Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    gXAudioMusicSourceVoice->lpVtbl->SubmitSourceBuffer(gXAudioMusicSourceVoice, &GameSound->Buffer, NULL);

    gXAudioMusicSourceVoice->lpVtbl->Start(gXAudioMusicSourceVoice, 0, XAUDIO2_COMMIT_NOW);
}

// Loads any defined asset type such as a *.wav, *.ogg, *.tmx, or *.bmpx from an asset file into heap memory.
// The asset file is a compressed zip archive. The asset file is created with a customized version of miniz.
// The only difference is that some of the zip file metadata constants were changed so that tools such as 7-zip, WinRAR, etc., 
// will not be able to recognize the file. The asset file currently does not support any directory structure.
DWORD LoadAssetFromArchive(_In_ char* ArchiveName, _In_ char* AssetFileName, _In_ RESOURCE_TYPE ResourceType, _Inout_ void* Resource)
{
    DWORD Error = ERROR_SUCCESS;

    mz_zip_archive Archive = { 0 };

    BYTE* DecompressedBuffer = NULL;

    size_t DecompressedSize = 0;

    BOOL FileFoundInArchive = FALSE;

    if ((mz_zip_reader_init_file(&Archive, ArchiveName, 0)) == FALSE)
    {
        Error = mz_zip_get_last_error(&Archive);        

        LogMessageA(LL_ERROR, "[%s] mz_zip_reader_init_file failed with 0x%08lx on archive file %s! Error: %s", __FUNCTION__, Error, ArchiveName, mz_zip_get_error_string(Error));

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] Archive %s opened.", __FUNCTION__, ArchiveName);

    // Iterate through each file in the archive until we find the file we are looking for.

    for (uint32_t FileIndex = 0; FileIndex < mz_zip_reader_get_num_files(&Archive); FileIndex++)
    {
        mz_zip_archive_file_stat CompressedFileStatistics = { 0 };

        if (mz_zip_reader_file_stat(&Archive, FileIndex, &CompressedFileStatistics) == MZ_FALSE)
        {
            Error = mz_zip_get_last_error(&Archive);            

            LogMessageA(LL_ERROR, "[%s] mz_zip_reader_file_stat failed with 0x%08lx! Archive: %s File: %s Error: %s", __FUNCTION__, Error, ArchiveName, AssetFileName, mz_zip_get_error_string(Error));

            goto Exit;
        }

        if (_stricmp(CompressedFileStatistics.m_filename, AssetFileName) == 0)
        {
            FileFoundInArchive = TRUE;

            if ((DecompressedBuffer = mz_zip_reader_extract_to_heap(&Archive, FileIndex, &DecompressedSize, 0)) == NULL)
            {
                Error = mz_zip_get_last_error(&Archive);                

                LogMessageA(LL_ERROR, "[%s] mz_zip_reader_extract_to_heap failed with 0x%08lx! Archive: %s File: %s Error: %s", __FUNCTION__, Error, ArchiveName, AssetFileName, mz_zip_get_error_string(Error));

                goto Exit;
            }

            LogMessageA(LL_INFO, "[%s] File %s found in asset file %s and extracted to heap.", __FUNCTION__, AssetFileName, ArchiveName);

            break;
        }
    }

    if (FileFoundInArchive == FALSE)
    {
        Error = ERROR_FILE_NOT_FOUND;

        LogMessageA(LL_ERROR, "[%s] File %s was not found in archive %s! 0x%08lx", __FUNCTION__, AssetFileName, ArchiveName, Error);

        goto Exit;
    }

    switch (ResourceType)
    {
        case RESOURCE_TYPE_WAV:
        {
            Error = LoadWavFromMemory(DecompressedBuffer, Resource);

            break;
        }
        case RESOURCE_TYPE_OGG:
        {
            Error = LoadOggFromMemory(DecompressedBuffer, (uint32_t)DecompressedSize, Resource);

            break;
        }
        case RESOURCE_TYPE_TILEMAP:
        {
            Error = LoadTilemapFromMemory(DecompressedBuffer, (uint32_t)DecompressedSize, Resource);

            break;
        }
        case RESOURCE_TYPE_BMPX:
        {
            Error = Load32BppBitmapFromMemory(DecompressedBuffer, Resource);

            break;
        }
        default:
        {
            ASSERT(FALSE, "Unknown resource type!");
        }
    }

Exit:    
        
    mz_zip_reader_end(&Archive);    

    return(Error);
}

DWORD AssetLoadingThreadProc(_In_ LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    DWORD Error = ERROR_SUCCESS;

    LogMessageA(LL_INFO, "[%s] Asset loading has begun.", __FUNCTION__);

    // The following resources are considered "essential" assets. They need to be loaded
    // first, as quickly as possible. Once the essential assets are loaded, set the event
    // to let the main thread know that at least the essential assets have been loaded,
    // even if we are not completely done loading all of the assets yet.
    if ((Error = LoadAssetFromArchive(ASSET_FILE, "6x7Font.bmpx", RESOURCE_TYPE_BMPX, &g6x7Font)) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading 6x7font.bmpx failed with 0x%08lx!", __FUNCTION__, Error);        

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "SplashScreen.wav", RESOURCE_TYPE_WAV, &gSoundSplashScreen)) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading SplashScreen.wav failed with 0x%08lx!", __FUNCTION__, Error);        

        goto Exit;
    }

    // End of "essential" assets.
    SetEvent(gEssentialAssetsLoadedEvent);

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Overworld01.bmpx", RESOURCE_TYPE_BMPX, &gOverworld01.GameBitmap)) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Overworld01.bmpx failed with 0x%08lx!", __FUNCTION__, Error);        

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Overworld01.tmx", RESOURCE_TYPE_TILEMAP, &gOverworld01.TileMap)) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Overworld01.tmx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "MenuNavigate.wav", RESOURCE_TYPE_WAV, &gSoundMenuNavigate)) != ERROR_SUCCESS)
    {       
        LogMessageA(LL_ERROR, "[%s] Loading MenuNavigate.wav failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "MenuChoose.wav", RESOURCE_TYPE_WAV, &gSoundMenuChoose)) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading MenuChoose.wav failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Overworld01.ogg", RESOURCE_TYPE_OGG, &gMusicOverworld01)) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Overworld01.ogg failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Down_Standing.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_DOWN_0])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Down_Standing.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Down_Walk1.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_DOWN_1])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Down_Walk1.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Down_Walk2.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_DOWN_2])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Down_Walk2.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Left_Standing.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_LEFT_0])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Left_Standing.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Left_Walk1.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_LEFT_1])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Left_Walk1.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Left_Walk2.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_LEFT_2])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Left_Walk2.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Right_Standing.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_RIGHT_0])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Right_Standing.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Right_Walk1.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_RIGHT_1])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Right_Walk1.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Right_Walk2.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_RIGHT_2])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Right_Walk2.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Up_Standing.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_UPWARD_0])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Up_Standing.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Up_Walk1.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_UPWARD_1])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Up_Walk1.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Error = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Up_Walk2.bmpx", RESOURCE_TYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_UPWARD_2])) != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Loading Hero_Suit0_Up_Walk2.bmpx failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }    

Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Asset loading has ended successfully.", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_INFO, "[%s] Asset loading has failed with result 0x%08lx!", __FUNCTION__, Error);
    }

    return(Error);
}