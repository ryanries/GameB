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
            TextColor.Red -= 64;

            TextColor.Green -= 64;

            TextColor.Blue -= 64;
        }

        if (LocalFrameCounter == 225)
        {
            TextColor.Red = 0;

            TextColor.Green = 0;

            TextColor.Blue = 0;
        }

        if (LocalFrameCounter >= 240)
        {
            gPreviousGameState = gCurrentGameState;

            gCurrentGameState = GAMESTATE_TITLESCREEN;
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
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_TITLESCREEN;
    }
}
