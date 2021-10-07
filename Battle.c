#include "Main.h"

#include "Battle.h"

MONSTER gCurrentMonster = { 0 };

const MONSTER gSlime001 = { "Slime", &gMonsterSprite_Slime_001, 5, 0, 10 };

const MONSTER gRat001 = { "Rat", &gMonsterSprite_Rat_001, 10, 0, 15 };

const MONSTER* gOutdoorMonsters[] = { &gSlime001, &gRat001 };

char gBattleTextLine1[64];

void GenerateMonster(void)
{
    unsigned int RandomValue = 0;

    rand_s(&RandomValue);

    // Make a copy of the monster for the player to fight; don't modify the template monster.
    memcpy(&gCurrentMonster, gOutdoorMonsters[RandomValue % _countof(gOutdoorMonsters)], sizeof(MONSTER));
    

    // if standing on outdoor tile then select from pool of outdoor monsters

    // if standing on dungeon tile, select from pool of dungeon monsters

    // etc...

    // Re-roll a new random value. If we reused the previous random value here,
    // the following text messages would always be coupled to the type of monster 
    // we previous rolled. We want to mix-and-match as much as possible.

    rand_s(&RandomValue);

    if (RandomValue % 2 == 0)
    {
        sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A wild %s approaches!", gCurrentMonster.Name);
    }
    else if (RandomValue % 3 == 0)
    {
        sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A %s crosses your path!", gCurrentMonster.Name);
    }
    else
    {
        sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A %s is coming right at you!", gCurrentMonster.Name);
    }    
}

void PPI_Battle(void)
{
	if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
	{
        ASSERT(gCurrentGameState == GAMESTATE_BATTLE, "Invalid game state!");        

		gPreviousGameState = gCurrentGameState;

		gCurrentGameState = GAMESTATE_OVERWORLD;

        LogMessageA(LL_DEBUG, "[%s] Transitioning from game state %d to %d. Player hit escape while in battle.",
            __FUNCTION__,
            gPreviousGameState,
            gCurrentGameState);

        StopMusic();
	}
}

void DrawBattle(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;    

    static int16_t BrightnessAdjustment = -255;

    static GAMEBITMAP* BattleScene = NULL;

    static uint16_t BattleTextLine1CharactersToShow = 0;

    char BattleTextLine1Scratch[64];

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

        BattleTextLine1CharactersToShow = 0;
    }

    // TODO: THIS IS BROKEN because if the player encounters a monster,
    // then hits escape to go back to the title screen, then goes back to the game,
    // a new monster will be generated mid-fight! So we can't rely on LocalFrameCounter == 0
    // Maybe we should just disable the escape key during battle?
    if (LocalFrameCounter == 0)
    {
        StopMusic();

        PlayGameMusic(&gMusicBattleIntro01, FALSE, TRUE);

        PlayGameMusic(&gMusicBattle01, TRUE, FALSE);
        
        GenerateMonster();

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
    }

    ApplyFadeIn(LocalFrameCounter, COLOR_NES_WHITE, &TextColor, &BrightnessAdjustment);
    
    BlitBackgroundToBuffer(&gOverworld01.GameBitmap, BrightnessAdjustment);

	//DrawWindow(0, 0, 96, 96, COLOR_NES_WHITE, WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_VERTICALLY_CENTERED | WINDOW_FLAG_BORDERED);



    // Draw the border around the monster battle scene.
    DrawWindow(0, 14, 100, 100,
        &TextColor,
        &COLOR_NES_BLACK,
        &COLOR_NES_BLACK,
        WINDOW_FLAG_OPAQUE | WINDOW_FLAG_BORDERED | WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_THICK | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_SHADOW);

    // Draw the battle scene, aka the backdrop behind the monster.
    if (BattleScene != 0)
    {
        Blit32BppBitmapToBuffer(BattleScene, 
            (int16_t)((GAME_RES_WIDTH / 2) - (BattleScene->BitmapInfo.bmiHeader.biWidth / 2)), 
            16, 
            BrightnessAdjustment);
    }
    else
    {
        ASSERT(FALSE, "BattleScene is NULL!");
    }

    // Draw the monster.
    if (gCurrentMonster.Sprite->Memory != NULL)
    {
        Blit32BppBitmapToBuffer(gCurrentMonster.Sprite, 
            (int16_t)((GAME_RES_WIDTH / 2) - (gCurrentMonster.Sprite->BitmapInfo.bmiHeader.biWidth / 2)), 
            48, 
            BrightnessAdjustment);
    } 
    
    // Draw the window where the scrolling text buffer will go.
    DrawWindow(0, 128, 256, 96,
        &TextColor,
        &COLOR_NES_BLACK,
        &COLOR_NES_BLACK,
        WINDOW_FLAG_OPAQUE | WINDOW_FLAG_BORDERED | WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_THICK | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_SHADOW);

    // Old fashioned typewriter animation for the text.
    if (LocalFrameCounter % 4 == 0)
    {
        if (BattleTextLine1CharactersToShow <= strlen(gBattleTextLine1))
        {
            BattleTextLine1CharactersToShow++;
        }
    }
    
    snprintf(BattleTextLine1Scratch, BattleTextLine1CharactersToShow, "%s", gBattleTextLine1);

    BlitStringToBuffer(BattleTextLine1Scratch, &g6x7Font, &TextColor, 67, 131);

    DrawPlayerStatsWindow(&TextColor);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}