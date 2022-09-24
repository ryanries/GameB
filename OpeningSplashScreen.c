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

#include "CommonMain.h"
#include "Platform.h"

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

    // The last frame we observed in this game state. This is in step with the global frame counter
    // as long as we are in this game state. If we leave this game state and come 
    // back to it later, we will know that because LastFrameSeen will be less than the 
    // global total number of frames rendered. We can use that fact to then reset any
    // local state for this gamestate so we can replay animations, reset text color, reset menus, etc.
    static uint64_t LastFrameSeen = 0;

    // AlphaAdjust is used to create a fade-out over time effect.
    static int AlphaAdjust = 0;

    // Blink is used to create the blinking effect for the little glyph at the bottom right-hand
    // corner of the splash screen which lets us know that the assets are still loading in the background.
    static bool Blink = false;

    // If we are not finished loading "essential" assets such as basic font and splash
    // screen noise, then exit immediately, because splash screen cannot be drawn yet.
    // LocalFrameCounter doesn't start counting until after this event is set.
    if (!EssentialAssetsAreLoaded())
    {
        // It's been over 30 seconds and the essential assets have not finished
        // loading yet. Something is wrong.
        if (gPerformanceData.TotalFramesRendered > (30*60))
        {
            LogMessageA(LL_ERROR, "[%s] Essential assets not loaded yet after 30 seconds. Unable to continue!", __FUNCTION__);

            #ifdef _WIN32
            MessageBoxA(gGameWindow, "Essential assets not loaded yet after 30 seconds. Unable to continue!", "Error", MB_OK | MB_ICONERROR);
            #endif

            exit(1);
        }

        return;
    }

    // If global TotalFramesRendered is greater than LastFrameSeen,
    // that means we have either just entered this gamestate for the first time,
    // or we have left this gamestate previously and have just come back to it.
    // For example we may have gone from the title screen, to the options screen,
    // and then back to the title screen again. In that case, we want to reset all
    // of the "local state," i.e., things that are local to this game state. Such
    // as text animation, selected menu item, etc.
    if (gPerformanceData.TotalFramesRendered == 0 ||
        gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;
        
        AlphaAdjust = 0;

        gInputEnabled = false;
    }

    // Clear the screen to black on each frame.
    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    // If assets are still being loaded in the background, then draw
    // a blinking cursor thingy at the bottom right of the screen to signify
    // that we are still busy loading assets in the background.
    if (!AssetThreadCompleted())
    {
        BlitStringEx(
            "Loading...", 
            &g6x7Font,             
            (GAME_RES_WIDTH - (6 * 11)), 
            (GAME_RES_HEIGHT - 7),
            32,
            32,
            32,
            0,
            0);

        if (Blink)
        {
            BlitStringEx(
                "\xf2", 
                &g6x7Font,
                (GAME_RES_WIDTH - 6), 
                (GAME_RES_HEIGHT - 7),
                32,
                32,
                32,
                0,
                0);
        }
    }
    else
    {
        if (gInputEnabled == false) 
        {
            // This allows the user to hit the Esc key to skip the rest of the 
            // opening splash screen, if they want.
            gInputEnabled = true; 
        }
    }

    // Alternate the "loading blinky cursor" every 0.5 seconds or 30 frames.
    if (gPerformanceData.TotalFramesRendered % 30 == 0)
    {
        Blink = !Blink;
    }

    // LocalFrameCounter is only incremented after the "essential" assets are loaded, i.e. after
    // the gEssentialAssetsLoadedEvent event is set. So none of the following will happen until
    // 90 frames after gEssentialAssetsLoadedEvent is set.
    if (LocalFrameCounter >= 90)
    {
        // Play the opening splash screen sound exactly once.
        if (LocalFrameCounter == 130)
        {
            PlayGameSound(&gSoundSplashScreen);
        }

        // Show the splash screen logo.

        Blit32BppBitmapEx( // The shadow
            &gPolePigLogo,
            (GAME_RES_WIDTH / 2) - (gPolePigLogo.BitmapInfo.bmiHeader.biWidth / 2),
            50 + 1,
            -192,
            -192,
            -192,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND);

        Blit32BppBitmapEx(
            &gPolePigLogo,
            (GAME_RES_WIDTH / 2) - (gPolePigLogo.BitmapInfo.bmiHeader.biWidth / 2),
            50,
            0,
            0,
            0,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND);

        // Show the lightning bolts as part of the splash screen logo,
        // attempt to synchronize it with the lightning sound.
        if ((LocalFrameCounter >= 130 && LocalFrameCounter < 140) ||
            (LocalFrameCounter >= 190 && LocalFrameCounter < 200) ||
            (LocalFrameCounter >= 270 && LocalFrameCounter < 280))
        {
            Blit32BppBitmapEx(
                &gLightning01,
                150,
                55,
                0,
                0,
                0,
                0,
                0);
        }

        if (LocalFrameCounter >= 280)
        {
            #ifdef SMOOTH_FADES

            AlphaAdjust -= 4;

            #else

            switch (LocalFrameCounter)
            {
                case 280:
                case 295:
                case 310:
                case 325:
                {
                    AlphaAdjust -= 64;
                }
            }

            #endif
        }

        // Splash screen animation is done, but we will linger here until the asset loading
        // background thread is finished.
        if (LocalFrameCounter >= 360)
        {
            if (AssetThreadCompleted())
            {
                if (AssetThreadFailed())
                {
                    LogMessageA(LL_ERROR, "[%s] Asset Loading Thread failed!", __FUNCTION__);

                    #ifdef _WIN32
                    MessageBoxA(gGameWindow, "Asset loading failed! Check log file for more details.", "Error", MB_OK | MB_ICONERROR);
                    #endif

                    exit(1);
                }

                gPreviousGameState = gCurrentGameState;

                gCurrentGameState = GAMESTATE_TITLESCREEN;

                LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Asset loading is complete.",
                    __FUNCTION__,
                    gPreviousGameState,
                    gCurrentGameState);
            }
        }

        BlitStringEx(
            "POLE PIG PRODUCTIONS",
            &g6x7Font,            
            (GAME_RES_WIDTH / 2) - ((20 * 6) / 2), 
            150,
            255,
            255,
            255,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
    }

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

// While the splash screen is playing, we allow the player to hit the Escape key
// to skip the opening splash screen and proceed directly to the title screen,
// BUT ONLY IF the background asset loading has completed and was successful.
void PPI_OpeningSplashScreen(void)
{
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        if (AssetThreadCompleted())
        {
            if (!AssetThreadFailed())
            {
                StopAllGameSounds();

                gPreviousGameState = gCurrentGameState;

                gCurrentGameState = GAMESTATE_TITLESCREEN;                

                LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Player pressed Escape to skip splash screen, and asset loading is complete.",
                    __FUNCTION__,
                    gPreviousGameState,
                    gCurrentGameState);
            }
            else
            {
                LogMessageA(LL_ERROR, "[%s] Asset loading thread failed!", __FUNCTION__);
                exit(1);
            }
        }
    }
}
