// Filename: OpeningSplashScreen.c
// Contains code for the functions that are specific to the opening splash screen.
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

#include "Main.h"

#include "OpeningSplashScreen.h"

void DrawOpeningSplashScreen(void)
{
    // The following static variables maintain their value across invocations of this function.
    // In other words, static variables persist like global variables except they can only be accessed
    // within the scope of this function.

    // The number of frames we have spent in this instance of this game state.
    // This counter resets to 0 if we leave this game state and come back to it later.
    // This is primarily used to control when things happen such as animations and text fade effects.
    static uint64_t LocalFrameCounter;

    // The last frame we observed in this game state. This is equal to the global frame counter
    // as long as we are in this game state. If we leave this game state and come 
    // back to it later, we will know that because LastFrameSeen will be less than the 
    // global total number of frames rendered. We can use that fact to then reset any
    // local state for this gamestate so we can replay animations, reset text color, etc.
    static uint64_t LastFrameSeen;

    // TextColor is used to create the fade in and fade out effect for the text.
    static PIXEL32 TextColor = { 0xFF, 0xFF, 0xFF, 0xFF };

    // Blink is used to create the blinking effect for the little glyph at the bottom right-hand
    // corner of the splash screen which lets us know that the assets are still loading in the
    // background.
    static BOOL Blink;

    // If we are not finished loading "essential" assets such as basic font and splash
    // screen noise, then exit immediately, because splash screen cannot be drawn yet.
    // LocalFrameCounter doesn't start counting until after this event is set.
    if (WaitForSingleObject(gEssentialAssetsLoadedEvent, 0) != WAIT_OBJECT_0)
    {
        // It's been over 5 seconds and the essential assets have not finished
        // loading yet. Something is wrong.
        if (gPerformanceData.TotalFramesRendered > 300)
        {
            LogMessageA(LL_ERROR, "[%s] Essential assets not loaded yet after 5 seconds. Unable to continue!", __FUNCTION__);

            gGameIsRunning = FALSE;

            MessageBoxA(gGameWindow, "Essential assets not loaded yet after 5 seconds. Unable to continue!", "Error", MB_OK | MB_ICONERROR);
        }

        return;
    }


    // Reset any state specific to this game state if this game state has been re-entered.
    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));
    }

    // Clear the screen to black on each frame.
    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    // If assets are still being loaded in the background, then draw
    // a blinking cursor thingy at the bottom right of the screen to signify
    // that we are still busy loading assets in the background.
    if (WaitForSingleObject(gAssetLoadingThreadHandle, 0) != WAIT_OBJECT_0)
    {
        BlitStringToBuffer("Loading...", &g6x7Font, &(PIXEL32) { 32, 32, 32, 255 }, 
            (GAME_RES_WIDTH - (6 * 11)), (GAME_RES_HEIGHT - 7));

        if (Blink)
        {
            BlitStringToBuffer("\xf2", &g6x7Font, &(PIXEL32) { 32, 32, 32, 255 },
                (GAME_RES_WIDTH - 6), (GAME_RES_HEIGHT - 7));
        }
    }

    // Alternate the "loading blinky cursor" every 0.5 seconds or 30 frames.
    if (gPerformanceData.TotalFramesRendered % 30 == 0)
    {
        Blink = !Blink;
    }

    // LocalFrameCounter is only incremented after the "essential" assets are loaded, i.e. after
    // the gEssentialAssetsLoadedEvent event is set. So none of the following will happen until
    // 120 frames after gEssentialAssetsLoadedEvent is set.
    if (LocalFrameCounter >= 120)
    {
        // Play the opening splash screen sound exactly once.
        if (LocalFrameCounter == 120)
        {
            PlayGameSound(&gSoundSplashScreen);
        }

        // Text pops in full white, then gradually fades to black.
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

        // Splash screen animation is done, but we will linger here until the asset loading
        // background thread is finished.
        if (LocalFrameCounter >= 240)
        {
            if (WaitForSingleObject(gAssetLoadingThreadHandle, 0) == WAIT_OBJECT_0)
            {
                DWORD ThreadExitCode = ERROR_SUCCESS;
                
                GetExitCodeThread(gAssetLoadingThreadHandle, &ThreadExitCode);

                if (ThreadExitCode != ERROR_SUCCESS)
                {
                    LogMessageA(LL_ERROR, "[%s] Asset Loading Thread failed with 0x%08lx!", __FUNCTION__, ThreadExitCode);

                    gGameIsRunning = FALSE;

                    MessageBoxA(gGameWindow, "Asset loading failed! Check log file for more details.", "Error", MB_OK | MB_ICONERROR);

                    return;
                }

                gPreviousGameState = gCurrentGameState;

                gCurrentGameState = GAMESTATE_TITLESCREEN;
            }
        }

        BlitStringToBuffer("-Game Studio-",
            &g6x7Font,
            &TextColor,
            (GAME_RES_WIDTH / 2) - ((13 * 6) / 2), 100);

        BlitStringToBuffer("Presents",
            &g6x7Font,
            &TextColor,
            (GAME_RES_WIDTH / 2) - ((8 * 6) / 2), 115);
    }

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void PPI_OpeningSplashScreen(void)
{
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        if (WaitForSingleObject(gAssetLoadingThreadHandle, 0) == WAIT_OBJECT_0)
        {
            gPreviousGameState = gCurrentGameState;

            gCurrentGameState = GAMESTATE_TITLESCREEN;
        }
    }
}
