// Filename: Main.c
// The GameCode module is designed to contain platform-independent game code. This means that the
// executable becomes the platform layer that interacts with the operating system, while this module
// can be used on different platforms (Windows, Linux, MacOS, etc.,) with minimal to no changes to the code.
// 
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
// Platform-independent game code binary.


// DllMain is required by the Windows OS, but should be #ifdef'ed out for other OSes.
#ifdef _WIN32

#pragma warning(push, 0)

#include <Windows.h>

#pragma warning(pop)


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
#endif