#include "Main.h"

#include "Battle.h"

void PPI_Battle(void)
{
	if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
	{
		gPreviousGameState = gCurrentGameState;

		gCurrentGameState = GAMESTATE_OVERWORLD;
	}
}

void DrawBattle(void)
{
	__stosd(gBackBuffer.Memory, 0xFF00FF00, GAME_DRAWING_AREA_MEMORY_SIZE / sizeof(DWORD));
}