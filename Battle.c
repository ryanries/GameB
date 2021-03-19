#include "Main.h"

#include "Battle.h"

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

    if (LocalFrameCounter == 10)
    {
        TextColor.Colors.Red = 64;

        TextColor.Colors.Green = 64;

        TextColor.Colors.Blue = 64;

        BrightnessAdjustment = -128;
    }

    if (LocalFrameCounter == 20)
    {
        TextColor.Colors.Red = 128;

        TextColor.Colors.Green = 128;

        TextColor.Colors.Blue = 128;

        BrightnessAdjustment = -64;
    }

    if (LocalFrameCounter == 30)
    {
        TextColor.Colors.Red = 192;

        TextColor.Colors.Green = 192;

        TextColor.Colors.Blue = 192;

        BrightnessAdjustment = -32;
    }

    if (LocalFrameCounter == 40)
    {
        TextColor.Colors.Red = 255;

        TextColor.Colors.Green = 255;

        TextColor.Colors.Blue = 255;

        BrightnessAdjustment = 0;

        gInputEnabled = TRUE;
    }

    BlitBackgroundToBuffer(&gOverworld01.GameBitmap, BrightnessAdjustment);

	DrawWindow(0, 0, 96, 96, (PIXEL32){ 0, 0, 0, 0xFF }, WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_VERTICALLY_CENTERED);


    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}