#include "Main.h"

#include "Battle.h"

MONSTER* gCurrentMonster = NULL;

MONSTER gSlime001 = { "Slime", &gMonsterSprite_Slime_001, 5, 0, 10 };

MONSTER gRat001 = { "Rat", &gMonsterSprite_Rat_001, 10, 0, 15 };

MONSTER* gOutdoorMonsters[] = { &gSlime001, &gRat001 };

void GenerateMonster(void)
{
    unsigned int RandomValue = 0;

    rand_s(&RandomValue);

    gCurrentMonster = gOutdoorMonsters[RandomValue % _countof(gOutdoorMonsters)];

    // if standing on outdoor tile then select from pool of outdoor monsters

    // if standing on dungeon tile, select from pool of dungeon monsters

    // etc...



    

    //gCurrentMonster = 
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

        // TODO: THIS IS BROKEN because if the player encounters a monster,
        // then hits escape to go back to the title screen, then goes back to the game,
        // a new monster will be generated mid-fight! So we can't rely on LocalFrameCounter == 0

        // TODO: ALSO we need to make copies of the random monsters we generate, not modify the 
        // archetypes themselves.
        GenerateMonster();

        if (gCurrentMonster == NULL)
        {
            ASSERT(FALSE, "No monster was generated!");
        }
    }

    ApplyFadeIn(LocalFrameCounter, COLOR_NES_WHITE, &TextColor, &BrightnessAdjustment);
    
    BlitBackgroundToBuffer(&gOverworld01.GameBitmap, BrightnessAdjustment);

	//DrawWindow(0, 0, 96, 96, COLOR_NES_WHITE, WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_VERTICALLY_CENTERED | WINDOW_FLAG_BORDERED);

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
    if (gCurrentMonster)
    {
        Blit32BppBitmapToBuffer(gCurrentMonster->Sprite, 
            (int16_t)((GAME_RES_WIDTH / 2) - (gCurrentMonster->Sprite->BitmapInfo.bmiHeader.biWidth / 2)), 
            48, 
            BrightnessAdjustment);
    } 
    
    // Draw the window where the scrolling text buffer will go.
    DrawWindow(0, 128, 256, 96,
        &TextColor,
        &COLOR_NES_BLACK,
        &COLOR_NES_BLACK,
        WINDOW_FLAG_OPAQUE | WINDOW_FLAG_BORDERED | WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_THICK | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_SHADOW);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}