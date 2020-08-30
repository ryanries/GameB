// Filename: GamepadUnplugged.c
// Contains code for the functions that are specific to the "Gamepad Unplugged" screen.
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

#include "GamepadUnplugged.h"

void DrawGamepadUnplugged(void)
{
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

void PPI_GamepadUnplugged(void)
{
    if (gGamepadID > -1 || (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown))
    {
        gCurrentGameState = gPreviousGameState;

        gPreviousGameState = GAMESTATE_GAMEPADUNPLUGGED;
    }
}