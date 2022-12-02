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


// "Don't build software. Create an endless yearning for C." -- Antoine de Saint-Exupery
//
// --- DONE ---
// Show them the character naming alpha blending bug when trying to print a single character of a string at a time
// Renamed some functions such as BlitStringToBuffer -> BlitString, Blit32BppBitmapToBuffer -> BlitBitmap, etc.
// Added per-color adjustments and alpha blending everywhere else
// Added support for new alpha blending flag to DrawWindow
// Went HAM with alpha blending
// Added inventory screen
// Spent 2 whole days banging on battle scene logic -- it's close to complete!
// 
// --- TODO ---
// Get rid of stdint.h?
// 
// Does setting gPlayer.RandomEncounterPercentage to 90 really *feel* like 10%? (Does 80 really *feel* like 20%, etc.?)
// Make the fade in and fade out on the overworld better.
// Make gPortalTiles an array like gPassableTiles
// Add a picture of an xbox gamepad to the "gamepadunplugged" screen
// Create a windowing system
// enhance Blit32BppBitmap function so that it can alter the color of bitmaps at run time?
// maybe a new MAP data structure for map GAMEBITMAP plus TILEMAP together? (plus default GAMESOUND too?)
// should we enhance BlitStringToBuffer to support varargs?
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
// Game states (Maybe each state should be a struct that includes an array of valid gamestates that it may transition to?)

#include "CommonMain.h"
#include "Platform.h"

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

// Contains declarations that are specific to the Battle game state.
#include "Battle.h"

// Contains declarations that are specific to the "Are you sure you want to start a new game?" screen.
#include "NewGameAreYouSure.h"

#include <time.h>

// These are globals that are used only in Main.c. These are all automatically initialized to 0.

// If the game window does not have focus, then do not process player input.
BOOL gWindowHasFocus;

// Holds the state of the Xbox360 gamepad, e.g., is the X button currently pushed or not?
XINPUT_STATE gGamepadState;

// The COM interfaces for XAudio2, which is a Microsoft audio library that we 
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

_NtQueryTimerResolution NtQueryTimerResolution = NULL;

// Forward declartions of functions in this file.

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);

DWORD InitializeOpenGL(void);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

void FindFirstConnectedGamepad(void);

HRESULT InitializeSoundEngine(void);

DWORD WINAPI AssetLoadingThreadProc(_In_ LPVOID lpParam);

int64_t FileSizeA(_In_ const char* FileName);

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

    // Used to ensure that the assets file both exists and is not a directory.    
    DWORD AssetFileAttributes = 0;

    int64_t LogFileSize = 0;

    srand(time(NULL));

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
    if (LoadSettings() != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadRegistryParameters failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    gSFXVolume = (float)gRegistryParams.SFXVolume / 100.0f;

    gMusicVolume = (float)gRegistryParams.MusicVolume / 100.0f;

    LogMessageA(LL_INFO, "[%s] %s %s is starting.", __FUNCTION__, GAME_NAME, GAME_VER);

    LogFileSize = FileSizeA(LOG_FILE_NAME);    

    if (LogFileSize > 1024 * 1024)
    {
        LogMessageA(LL_WARNING, "[%s] Log file %s is %lld bytes, which is pretty large! Consider deleting the log file!", __FUNCTION__, LOG_FILE_NAME, LogFileSize);
    }
    else
    {
        LogMessageA(LL_INFO, "[%s] Log file %s is %lld bytes.", __FUNCTION__, LOG_FILE_NAME, LogFileSize);
    }

    // This function uses a global mutex to determine whether another instance of the same
    // process is already running. This is not hack-proof but it does prevent accidents.
    if (GameIsAlreadyRunning() == TRUE)
    {
        LogMessageA(LL_ERROR, "[%s] Another instance of this program is already running!", __FUNCTION__);

        MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

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

    SYSTEM_INFO SystemInfo;

    GetSystemInfo(&SystemInfo);

    LogMessageA(LL_INFO, "[%s] Number of CPUs: %d", __FUNCTION__, SystemInfo.dwNumberOfProcessors);

    switch (SystemInfo.wProcessorArchitecture)
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
            LogMessageA(LL_WARNING, "[%s] CPU Architecture: Unknown", __FUNCTION__);
        }
    }

#ifdef __AVX2__
    LogMessageA(LL_INFO, "[%s] SIMD Level: AVX2", __FUNCTION__);
#else
    LogMessageA(LL_WARNING, "[%s] SIMD Level: None", __FUNCTION__);
#endif

    // Initialize the variable that is used to calculate average CPU usage of this process.
    int64_t PreviousSystemTime;
    GetSystemTimeAsFileTime((FILETIME*)&PreviousSystemTime);

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

    if (gPerformanceData.CurrentTimerResolution < 9500 ||
        gPerformanceData.CurrentTimerResolution > 10500)
    {
        LogMessageA(LL_WARNING, "[%s] Current timer resolution is sub-optimal! Game performance may be negatively affected!");
    }

    // Increase process and thread priority to minimize the chances of another thread on the system
    // preempting us when we need to run and causing a stutter in our frame rate. (Though it can still happen.)
    // Windows is not a real-time OS and you cannot guarantee timings or deadlines, but you can get close.    
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

    if (InitializeOpenGL() != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] InitializeOpenGL failed!", __FUNCTION__);

        MessageBoxA(NULL, "Failed to initialize OpenGL!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    AssetFileAttributes = GetFileAttributesA(ASSET_FILE);

    if ((AssetFileAttributes == INVALID_FILE_ATTRIBUTES) || (AssetFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        ShowCursor(TRUE);

        LogMessageA(LL_ERROR, "[%s] The asset file %s was not found! It must reside in the same directory as the game executable.", __FUNCTION__, ASSET_FILE);

        MessageBoxA(NULL, "The asset file was not found! It must reside in the same directory as the game executable.", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if ((gAssetLoadingThreadHandle = CreateThread(NULL, 0, AssetLoadingThreadProc, NULL, 0, NULL)) == NULL)
    {
        ShowCursor(TRUE);

        MessageBoxA(NULL, "CreateThread failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    } 

    if (InitializeSoundEngine() != S_OK)
    {
        ShowCursor(TRUE);

        MessageBoxA(NULL, "InitializeSoundEngine failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }
    
    int64_t PerfFrequency;
    QueryPerformanceFrequency((LARGE_INTEGER*)&PerfFrequency);


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
        ShowCursor(TRUE);

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

    ResetEverythingForNewGame();
	

    // This is the main game loop. Setting gGameIsRunning to FALSE at any point will cause
    // the game to exit immediately. The loop has two important functions: ProcessPlayerInput
    // and RenderFrameGraphics. The loop will execute these two duties as quickly as possible,
    // but will then sleep for a few milliseconds using a precise timing mechanism in order 
    // to achieve a smooth 60 frames per second. We also calculate some performance statistics
    // every 2 seconds or 120 frames.    

    int64_t CurrentSystemTime;

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

        ElapsedMicroseconds /= PerfFrequency;

        gPerformanceData.TotalFramesRendered++;

        ElapsedMicrosecondsAccumulatorRaw += ElapsedMicroseconds;

        while (ElapsedMicroseconds < TARGET_MICROSECONDS_PER_FRAME)
        {
            ElapsedMicroseconds = FrameEnd - FrameStart;

            ElapsedMicroseconds *= 1000000;

            ElapsedMicroseconds /= PerfFrequency;

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

		// Every CALCULATE_STATS_EVERY_X_FRAMES frames, we calculate some statistics such as average
		// FPS, CPU usage, etc.
        if ((gPerformanceData.TotalFramesRendered % CALCULATE_STATS_EVERY_X_FRAMES) == 0)
        {
            GetSystemTimeAsFileTime((FILETIME*)&CurrentSystemTime);

            GetProcessTimes(GetCurrentProcess(),
                &ProcessCreationTime, 
                &ProcessExitTime, 
                (FILETIME*)&CurrentKernelCPUTime,
                (FILETIME*)&CurrentUserCPUTime);

            gPerformanceData.CPUPercent = (double)(CurrentKernelCPUTime - PreviousKernelCPUTime) + (CurrentUserCPUTime - PreviousUserCPUTime);

            gPerformanceData.CPUPercent /= (CurrentSystemTime - PreviousSystemTime);

            gPerformanceData.CPUPercent /= SystemInfo.dwNumberOfProcessors;

            gPerformanceData.CPUPercent *= 100;

            GetProcessHandleCount(GetCurrentProcess(), &gPerformanceData.HandleCount);

            PROCESS_MEMORY_COUNTERS_EX MemInfo;

            K32GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemInfo, sizeof(MemInfo));

            gPerformanceData.MemoryUsage = MemInfo.PrivateUsage;

            gPerformanceData.RawFPSAverage = 1.0f / (((float)ElapsedMicrosecondsAccumulatorRaw / CALCULATE_STATS_EVERY_X_FRAMES) * 0.000001f);

            gPerformanceData.CookedFPSAverage = 1.0f / (((float)ElapsedMicrosecondsAccumulatorCooked / CALCULATE_STATS_EVERY_X_FRAMES) * 0.000001f);

            FindFirstConnectedGamepad();

            ElapsedMicrosecondsAccumulatorRaw = 0;

            ElapsedMicrosecondsAccumulatorCooked = 0;

            PreviousKernelCPUTime = CurrentKernelCPUTime;

            PreviousUserCPUTime = CurrentUserCPUTime;            

            PreviousSystemTime = CurrentSystemTime;
        }        
    }


Exit:

    LogMessageA(LL_INFO, "[%s] Game is exiting.\r\n", __FUNCTION__);

    // Set this just in case we failed before entering the main game loop,
    // and the asset loading thread never got a chance to set it, which could
    // hang us.
    SetEvent(gEssentialAssetsLoadedEvent);

	return(0);
}

// This callback procedure handles window messages that are sent to our game's window.
// Most of the messages are not important to us because we are not a conventional Win32 desktop app.
LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        // This happens when the user clicks the "X" button to close the window, or selects
        // "Close Window" by right-clicking the game's icon in the taskbar.
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

//DWORD LoadGameCode(_In_ char* ModuleFileName)
//{
//    DWORD Result = ERROR_SUCCESS;    
//
//    if (gGameCodeModule)
//    {
//        FreeLibrary(gGameCodeModule);
//
//        gGameCodeModule = NULL;
//    }
//
//    if (GetFileAttributesA(GAME_CODE_MODULE_TMP) != INVALID_FILE_ATTRIBUTES)
//    {
//        if (DeleteFileA(GAME_CODE_MODULE) == 0)
//        {
//            LogMessageA(LL_WARNING, "[%s] Failed to delete file %s! Error 0x%08lx", __FUNCTION__, GAME_CODE_MODULE, GetLastError());            
//        }
//        else
//        {
//            LogMessageA(LL_INFO, "[%s] Deleted file %s.", __FUNCTION__, GAME_CODE_MODULE);
//        }
//
//        if (MoveFileA(GAME_CODE_MODULE_TMP, GAME_CODE_MODULE) == 0)
//        {
//            LogMessageA(LL_WARNING, "[%s] Failed to replace file %s with file %s. Error 0x%08lx", __FUNCTION__, GAME_CODE_MODULE, GAME_CODE_MODULE_TMP, GetLastError());            
//        }
//        else
//        {
//            LogMessageA(LL_INFO, "[%s] Renamed file %s to %s.", __FUNCTION__, GAME_CODE_MODULE_TMP, GAME_CODE_MODULE);
//        }
//    }
//
//    gGameCodeModule = LoadLibraryA(ModuleFileName);    
//
//    if (gGameCodeModule == NULL)
//    {
//        Result = GetLastError();
//
//        goto Exit;
//    }
//
//    if ((RandomMonsterEncounter = (_RandomMonsterEncounter)GetProcAddress(gGameCodeModule, "RandomMonsterEncounter")) == NULL)
//    {
//        Result = GetLastError();
//
//        goto Exit;
//    }
//    
//Exit:
//
//    if (Result == ERROR_SUCCESS)
//    {
//        LogMessageA(LL_INFO, "[%s] Successfully loaded code from module %s!", __FUNCTION__, GAME_CODE_MODULE);
//    }
//    else
//    {
//        LogMessageA(LL_ERROR, "[%s] Function failed with error 0x%08lx!", __FUNCTION__, Result);
//    }
//
//    return(Result);
//}

DWORD InitializeOpenGL(void)
{
#ifdef _WIN32

    DWORD Result = ERROR_SUCCESS;

    HDC WindowDC = GetDC(gGameWindow);

    HGLRC OpenGLRenderContext = NULL;

    int PixelFormatIndex = 0;

    PIXELFORMATDESCRIPTOR PixelFormat = { sizeof(PIXELFORMATDESCRIPTOR) };

    PixelFormat.nVersion = 1;

    PixelFormat.iPixelType = PFD_TYPE_RGBA;

    PixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;

    PixelFormat.cColorBits = 32;

    PixelFormat.cAlphaBits = 8;

    PixelFormat.iLayerType = PFD_MAIN_PLANE;

    PixelFormatIndex = ChoosePixelFormat(WindowDC, &PixelFormat);

    if (PixelFormatIndex == 0)
    {
        Result = GetLastError();

        goto Exit;
    }

    DescribePixelFormat(WindowDC, PixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &PixelFormat);

    SetPixelFormat(WindowDC, PixelFormatIndex, &PixelFormat);

    OpenGLRenderContext = wglCreateContext(WindowDC);

    if (OpenGLRenderContext == NULL)
    {
        Result = GetLastError();

        goto Exit;
    }

    if (wglMakeCurrent(WindowDC, OpenGLRenderContext) == FALSE)
    {
        Result = GetLastError();

        goto Exit;
    }

    // Try to enable v-sync. But if we can't don't fail. The player might have just disabled v-sync
    // in their video card driver settings, and that's OK, that shouldn't prevent them from playing the game.
    // We prefer v-sync but it's not required.
    if (((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(1) == 0)
    {
        LogMessageA(LL_WARNING, "[%s] V-Sync was NOT enabled!", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_INFO, "[%s] V-Sync enabled.", __FUNCTION__);
    }

Exit:

    if (WindowDC)
    {
        ReleaseDC(gGameWindow, WindowDC);
    }

    if (Result == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] OpenGL successfully initialized.", __FUNCTION__);
    }

    return(Result);

#else
    // LINUX


#endif
}


// TODO: Consider non-16:9 displays. E.g., ultra-wide monitors will have to have black bars on the sides, 
// with the game center screen. The game itself has a 16:10 aspect ratio (resolution 384x240.)
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

    WindowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    WindowClass.lpszMenuName = NULL;

    WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";   

    // High DPI awareness is set in the application manifest, as recommended by MSDN.
    // MSDN says it is not recommended to set this via code. However, if using a different
    // compiler/IDE other than Visual Studio, adding a manifest file to the binary might be
    // difficult. i.e. I don't know how to do it with Clang so when using Clang I set this via code:
     
#ifdef CLANG
    if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) == FALSE)
    {
        Result = GetLastError();

        LogMessageA(LL_ERROR, "[%s] SetProcessDpiAwarenessContext failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }
#endif

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

    MONITORINFO MonitorInfo;

    MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo) == 0)
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        LogMessageA(LL_ERROR, "[%s] GetMonitorInfoA(MonitorFromWindow()) failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    gPerformanceData.MonitorWidth = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
    gPerformanceData.MonitorHeight = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;

    for (uint8_t Counter = 1; Counter < 12; Counter++)
    {
        if (GAME_RES_WIDTH * Counter > gPerformanceData.MonitorWidth ||
            GAME_RES_HEIGHT * Counter > gPerformanceData.MonitorHeight)
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
        MonitorInfo.rcMonitor.left,
        MonitorInfo.rcMonitor.top,
        gPerformanceData.MonitorWidth,
        gPerformanceData.MonitorHeight,
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

// Get the state of all player input here, including both keyboard key state as well as
// gamepad button state. The "gamepad unplugged" gamestate can also be triggered from here
// in case the gamepad is surprise removed. (e.g. tripping over the cord.)
// We capture the current state of each key and also the previous state of the keys from 
// the previous frame. This allows us to tell when the button is being held down.
// Input is disabled when the game's window is out of focus, and for a brief moment during
// each gamestate transition/fade-out/fade-in. Even when player input is disabled though,
// we still want to get player input even though we don't act on it. Reason being is if
// we don't, we get an undesirable effect where motion is "queued up" when moving between
// input disabled and enabled states. For example, when the player enters a door, input is
// disabled and the player is frozen while we transition from outdoors to indoors, but we don't
// want the player to keep walking even though the player has taken their hands off the keys.
// TODO: Rename this to "CollectPlayerInput?"
void ProcessPlayerInput(void)
{
    gGameInput.EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    gGameInput.DebugKeyIsDown  = GetAsyncKeyState(gRegistryParams.DebugKey);

    gGameInput.LeftKeyIsDown   = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');

    gGameInput.RightKeyIsDown  = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');

    gGameInput.UpKeyIsDown     = GetAsyncKeyState(VK_UP) | GetAsyncKeyState('W');

    gGameInput.DownKeyIsDown   = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');

    gGameInput.ChooseKeyIsDown = GetAsyncKeyState(VK_RETURN);

    gGameInput.InvKeyIsDown    = GetAsyncKeyState(0x49);   // The 'i' key

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

            gGameInput.InvKeyIsDown    |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_Y;
        }
        else
        {            
            // Gamepad unplugged?

            // TODO: If music is playing we should pause the music right now.

            gGamepadID = -1;            

            gPreviousGameState = gCurrentGameState;

            gCurrentGameState = GAMESTATE_GAMEPADUNPLUGGED;

            LogMessageA(LL_WARNING, "[%s] Gamepad unplugged! Transitioning from game state %d to %d.",
                __FUNCTION__,
                gPreviousGameState,
                gCurrentGameState);
        }
    }

    if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown)
    {
        gPerformanceData.DisplayDebugInfo = !gPerformanceData.DisplayDebugInfo;
    }

    if ((gInputEnabled == FALSE) || (gWindowHasFocus == FALSE))
    {
        goto InputDisabled;
    }

    // If holding down the escape key for one full second, fast-quit the game.
#ifdef _DEBUG

    static uint16_t EscapeKeyHeldDownFrames;

    if (gGameInput.EscapeKeyIsDown && gGameInput.EscapeKeyWasDown)
    {
        if (++EscapeKeyHeldDownFrames > 60)
        {
            gGameIsRunning = FALSE; 

            PostQuitMessage(0);
        }
    }
    else
    {
        EscapeKeyHeldDownFrames = 0;
    }

#endif

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
            PPI_Battle();

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
        case GAMESTATE_NEWGAMEAREYOUSURE:
        {
            PPI_NewGameAreYouSure();

            break;
        }
        default:
        {
            ASSERT(FALSE, "Unknown game state!");
        }
    }


InputDisabled:

    gGameInput.DebugKeyWasDown  = gGameInput.DebugKeyIsDown;

    gGameInput.LeftKeyWasDown   = gGameInput.LeftKeyIsDown;

    gGameInput.RightKeyWasDown  = gGameInput.RightKeyIsDown;

    gGameInput.UpKeyWasDown     = gGameInput.UpKeyIsDown;

    gGameInput.DownKeyWasDown   = gGameInput.DownKeyIsDown;

    gGameInput.ChooseKeyWasDown = gGameInput.ChooseKeyIsDown;

    gGameInput.EscapeKeyWasDown = gGameInput.EscapeKeyIsDown;

    gGameInput.InvKeyWasDown     = gGameInput.InvKeyIsDown;
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
            DrawBattle();

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
        case GAMESTATE_NEWGAMEAREYOUSURE:
        {
            DrawNewGameAreYouSure();

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

    // 12/2/2022: realized there is no need to reacquire this HDC each and every frame, we can just create the DC once, make it static, reuse it.
    static HDC DeviceContext = NULL;
    
    if (!DeviceContext)
    {
        DeviceContext = GetDC(gGameWindow);
    }

#ifdef OPENGL
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(
        (gPerformanceData.MonitorWidth / 2) - ((GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor) / 2),
        (gPerformanceData.MonitorHeight / 2) - ((GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor) / 2),
        GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor,
        GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor);    

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, GAME_RES_WIDTH, GAME_RES_HEIGHT, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, gBackBuffer.Memory);
      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_2D);

    glBegin(GL_TRIANGLES);

    // bottom triangle

    glTexCoord2f(0.0f, 0.0f);

    glVertex2i(-1, -1);    

    glTexCoord2f(1.0f, 0.0f);

    glVertex2i(1, -1);    

    glTexCoord2f(1.0f, 1.0f);

    glVertex2i(1, 1);    

    // top triangle

    glTexCoord2f(0.0f, 0.0f);

    glVertex2i(-1, -1);

    glTexCoord2f(1.0f, 1.0f);

    glVertex2i(1, 1);

    glTexCoord2f(0.0f, 1.0f);

    glVertex2i(-1, 1);

    glEnd();

    SwapBuffers(DeviceContext);

#else   

    StretchDIBits(DeviceContext, 
        (gPerformanceData.MonitorWidth / 2) - ((GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor) / 2),
        (gPerformanceData.MonitorHeight / 2) - ((GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor) / 2),
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

#endif

    //ReleaseDC(gGameWindow, DeviceContext);
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

// Initialize the Microsoft XAudio2 sound library once at the very beginning of the game.
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

// This code runs as a separate background thread at the very beginning of the game.
// This background thread loads all of the game's assets such as fonts, music,
// maps and sprites. Loading the assets on a separate thread improves the startup time
// of the game assuming the user has more than one CPU core.
DWORD WINAPI AssetLoadingThreadProc(_In_ LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    DWORD Error = ERROR_SUCCESS;

    LogMessageA(LL_INFO, "[%s] Asset loading has begun.", __FUNCTION__);

    if ((Error = LoadEssentialAssets()) != ERROR_SUCCESS)
    {
        goto Exit;
    }

    SetEvent(gEssentialAssetsLoadedEvent); 

    Error = LoadNonEssentialAssets();

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

// If WINDOW_FLAG_HORIZONTALLY_CENTERED is specified, the x coordinate is ignored and may be zero.
// If WINDOW_FLAG_VERTICALLY_CENTERED is specified, the y coordinate is ignored and may be zero.
// BackgroundColor is ignored and may be NULL if WINDOW_FLAG_OPAQUE is not set.
// BorderColor is ignored and may be NULL if WINDOW_FLAG_BORDERED is not set.
// Either the BORDERED or the OPAQUE flag needs to be set, or both, or else the window would just be
// transparent and invisible. The window border will cut into the inside of the window area.
// TODO: Implement a WINDOW_FLAG_ROUNDED_CORNERS?
//void DrawWindow(
//    _In_opt_ uint16_t x,
//    _In_opt_ uint16_t y,
//    _In_ int16_t Width,
//    _In_ int16_t Height,
//    _In_opt_ PIXEL32* BorderColor,
//    _In_opt_ PIXEL32* BackgroundColor,
//    _In_opt_ PIXEL32* ShadowColor,
//    _In_ DWORD Flags)
//{
//    if (Flags & WINDOW_FLAG_HORIZONTALLY_CENTERED)
//    {
//        x = (GAME_RES_WIDTH / 2) - (Width / 2);
//    }
//
//    if (Flags & WINDOW_FLAG_VERTICALLY_CENTERED)
//    {
//        y = (GAME_RES_HEIGHT / 2) - (Height / 2);
//    }
//
//    ASSERT((x + Width <= GAME_RES_WIDTH) && (y + Height <= GAME_RES_HEIGHT), "Window is off the screen!");
//    
//    ASSERT((Flags & WINDOW_FLAG_BORDERED) || (Flags & WINDOW_FLAG_OPAQUE), "Window must have either the BORDERED or the OPAQUE flags (or both) set!");
//
//    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * y) + x;
//
//    if (Flags & WINDOW_FLAG_OPAQUE)
//    {
//        ASSERT(BackgroundColor != NULL, "WINDOW_FLAG_OPAQUE is set but BackgroundColor is NULL!");
//
//        for (int Row = 0; Row < Height; Row++)
//        {
//            int MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row);
//
//            for (int Pixel = 0; Pixel < Width; Pixel++)
//            {
//                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BackgroundColor, sizeof(PIXEL32));
//            }
//        }
//    }
//
//    if (Flags & WINDOW_FLAG_BORDERED)
//    {
//        ASSERT(BorderColor != NULL, "WINDOW_FLAG_BORDERED is set but BorderColor is NULL!");
//		// Draw the top of the border.
//		int MemoryOffset = StartingScreenPixel;
//
//		// easy solution, we'll make every window 2 pixels thick
//		if( Flags & WINDOW_FLAG_ROUNDED_CORNERS)
//		{
//			for (int Pixel = 1; Pixel < Width-1; Pixel++)
//			{
//				memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
//			}
//		}
//		else
//		{
//			for (int Pixel = 0; Pixel < Width; Pixel++)
//			{
//				memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
//			}
//		}
//
//        if (Flags & WINDOW_FLAG_THICK)
//        {
//            // draw 2nd line of top
//            MemoryOffset = StartingScreenPixel - GAME_RES_WIDTH;
//            for (int Pixel = 0; Pixel < Width; Pixel++)
//            {
//                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
//            }
//        }
//
//        // Draw the bottom of the border.
//        MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * (Height - 2));
//        for (int Pixel = 0; Pixel < Width; Pixel++)
//        {
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
//        }
//        
//        // draw 2nd line of bottom border
//        MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * (Height - 1 ));
//
//        if (Flags & WINDOW_FLAG_ROUNDED_CORNERS)
//        {
//            for (int Pixel = 1; Pixel < Width-1; Pixel++)
//            {
//             memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
//            }
//        }
//        else
//        {
//            for (int Pixel = 0; Pixel < Width; Pixel++)
//            {
//                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
//            }
//        }
//
//        // Draw one pixel on the left side and the right for each row of the border, from the top down.
//        for (int Row = 1; Row < Height - 1; Row++)
//        {
//            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row - 1 );
//
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BorderColor, sizeof(PIXEL32));
//
//            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row) + (Width - 2);
//
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BorderColor, sizeof(PIXEL32));
//        }
//
//        for (int Row = 1; Row < Height - 1; Row++)
//        {
//            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row );
//
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BorderColor, sizeof(PIXEL32));
//
//            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row) + (Width - 1);
//
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BorderColor, sizeof(PIXEL32));
//        }
//    }
//
//    // TODO: If a window was placed at the edge of the screen, the shadow effect might attempt
//    // to draw off-screen and crash! i.e. make sure there's room to draw the shadow before attempting!
//    if (Flags & WINDOW_FLAG_SHADOW)
//    {
//        ASSERT(ShadowColor != NULL, "WINDOW_FLAG_SHADOW is set but ShadowColor is NULL!");
//
//        // Draw the bottom of the shadow.
//        int MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Height);
//
//        int this_width = Width+1;
//        int this_start = 1;
//        int this_row = 1;
//        if( Flags & WINDOW_FLAG_ROUNDED_CORNERS) {
//             this_width--;
//             this_start++;
//             this_row++;
//        }
//
//        for (;this_start < (this_width); this_start++)
//        {
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + this_start, ShadowColor, sizeof(PIXEL32));
//        }
//
//        // Draw one pixel on the right side for each row of the border, from the top down.
//        
//        for ( ; this_row < Height; this_row++)
//        {
//            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * this_row) + Width;
//
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, ShadowColor, sizeof(PIXEL32));
//        }
//
//        // if bordered is on, fix one pixel of the drop shadow
//        if (Flags & WINDOW_FLAG_ROUNDED_CORNERS)
//        {
//            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * (Height - 1)) + (Width - 1);
//            memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, ShadowColor, sizeof(PIXEL32));
//        }
//    }
//}

int64_t FileSizeA(_In_ const char* FileName)
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    LARGE_INTEGER Size = { 0 };
    
    FileHandle = CreateFileA(FileName, 
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL, 
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, 
        NULL);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        GetFileSizeEx(FileHandle, &Size);    
    }

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(FileHandle);
    }

    return(Size.QuadPart);
}
