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