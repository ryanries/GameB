#include "Main.h"

#include "Battle.h"

void GenerateMonster(void)
{
    
}

void PPI_Battle(void)
{
	if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
	{
		gPreviousGameState = gCurrentGameState;

		gCurrentGameState = GAMESTATE_OVERWORLD;

        StopMusic();
	}
}

void DrawBattle(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;    

    static int16_t BrightnessAdjustment = -255;

    GAMEBITMAP* BattleScene = NULL;

    // If global TotalFramesRendered is greater than LastFrameSeen,
    // that means we have either just entered this gamestate for the first time,
    // or we have left this gamestate previously and have just come back to it.
    // For example we may have gone from the title screen, to the options screen,
    // and then back to the title screen again. In that case, we want to reset all
    // of the "local state," i.e., things that are local to this game state. Such
    // as text animation, selected menu item, etc.
    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));

        gInputEnabled = FALSE;
    }

    if (LocalFrameCounter == 0)
    {
        StopMusic();

        PlayGameMusic(&gMusicBattleIntro01, FALSE, TRUE);

        PlayGameMusic(&gMusicBattle01, TRUE, FALSE);
    }

    ApplyFadeIn(LocalFrameCounter, COLOR_NES_WHITE, &TextColor, &BrightnessAdjustment);
    
    BlitBackgroundToBuffer(&gOverworld01.GameBitmap, BrightnessAdjustment);

	DrawWindow(0, 0, 96, 96, COLOR_NES_WHITE, WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_VERTICALLY_CENTERED | WINDOW_FLAG_BORDERED);

    switch (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16])
    {
        case TILE_GRASS_01:
        {
            BattleScene = &gBattleScene_Grasslands01;

            break;
        }
        case TILE_BRICK_01:
        {
            BattleScene = &gBattleScene_Dungeon01;

            break;
        }
        default:
        {
            ASSERT(FALSE, "Random monster encountered on an unknown tile!");
        }
    }

    if (BattleScene != 0)
    {
        Blit32BppBitmapToBuffer(BattleScene, 145, 73, BrightnessAdjustment);
    }
    else
    {
        ASSERT(FALSE, "BattleScene is NULL!");
    }

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}