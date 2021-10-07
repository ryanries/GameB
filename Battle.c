#include "Main.h"

#include "Battle.h"

// This holds a copy of one of the monster templates.
MONSTER gCurrentMonster = { 0 };

const MONSTER gSlime001 = { 
    "Slime",                    // Name
    &gMonsterSprite_Slime_001,  // Sprite
    5,                          // BaseHP
    0,                          // BaseMP
    10,                         // BaseXP
    1,                          // BaseDamage
    { "*jiggles menacingly*",   // Emotes
      "*spits and gurgles*",
      "*thousand-yard stare*" }
};

const MONSTER gRat001 = { 
    "Rat", 
    &gMonsterSprite_Rat_001, 
    10, 
    0, 
    15,
    1,
    { "*squeak squeak*",
      "*whiskers twitching angrily*",
      "*looks like it might have rabies*" }
};

const MONSTER* gOutdoorMonsters[] = { &gSlime001, &gRat001 };

// A random string like "A wild %s draws near!"
char gBattleTextLine1[64];

// The second line of text, whether this is a surprise attack or not.
char gBattleTextLine2[64];

// Third line of battle text, this is a randomly selected monster emote.
char gBattleTextLine3[64];

// Is this a "surprise attack" where the monster gets the first move.
BOOL gMonsterGoesFirst;

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

    switch (RandomValue % 5)
    {
        case 0:
        {
            sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A wild %s approaches!", gCurrentMonster.Name);

            break;
        }
        case 1:
        {
            sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A %s crosses your path!", gCurrentMonster.Name);

            break;
        }
        case 2:
        {
            sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A mean-looking %s lurches toward you!", gCurrentMonster.Name);

            break;
        }
        case 3:
        {
            sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A %s is coming right at you!", gCurrentMonster.Name);

            break;
        }
        case 4:
        {
            sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "A %s jumps out at you!", gCurrentMonster.Name);

            break;
        }
        default:
        {
            ASSERT(FALSE, "RandomValue mod X exceeded the number of random battle texts!");
        }
    }

    ASSERT(strlen(gBattleTextLine1) > 0, "Error generating battle text!")


    // Re-roll another random value to determine whether the monster
    // gets to attack first or not.
    rand_s(&RandomValue);

    if (RandomValue % 3 == 0)
    {
        gMonsterGoesFirst = TRUE;

        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You are caught off guard!");
    }
    else
    {
        gMonsterGoesFirst = FALSE;

        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You are ready for battle.");
    }

    ASSERT(strlen(gBattleTextLine2) > 0, "Error generating battle text!")

    // Re-roll to select a random monster emote.
    rand_s(&RandomValue);

    sprintf_s(gBattleTextLine3, 
        sizeof(gBattleTextLine3), 
        "%s: %s", gCurrentMonster.Name, gCurrentMonster.Emotes[(RandomValue % 3)]);

    ASSERT(strlen(gBattleTextLine3) > 0, "Error generating battle text!")
    
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


    // These strings are "scratch space" to display only portions
    // of the full line of text. This is used to make a "typewriter" like animation.

    static uint16_t BattleTextLine1CharactersToShow = 0;

    static uint16_t BattleTextLine2CharactersToShow = 0;

    static uint16_t BattleTextLine3CharactersToShow = 0;

    char BattleTextLine1Scratch[64] = { 0 };

    char BattleTextLine2Scratch[64] = { 0 };

    char BattleTextLine3Scratch[64] = { 0 };

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

        BattleTextLine2CharactersToShow = 0;

        BattleTextLine3CharactersToShow = 0;
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
    if (LocalFrameCounter % 3 == 0)
    {
        if (BattleTextLine1CharactersToShow <= strlen(gBattleTextLine1))
        {
            BattleTextLine1CharactersToShow++;
        }
    }
    
    snprintf(BattleTextLine1Scratch, BattleTextLine1CharactersToShow, "%s", gBattleTextLine1);

    BlitStringToBuffer(BattleTextLine1Scratch, &g6x7Font, &TextColor, 67, 131);

    // Don't start drawing line 2 until line 1 is finished animating.
    if (strlen(BattleTextLine1Scratch) == strlen(gBattleTextLine1))
    {
        // Old fashioned typewriter animation for the text.
        if (LocalFrameCounter % 3 == 0)
        {
            if (BattleTextLine2CharactersToShow <= strlen(gBattleTextLine2))
            {
                BattleTextLine2CharactersToShow++;
            }
        }

        snprintf(BattleTextLine2Scratch, BattleTextLine2CharactersToShow, "%s", gBattleTextLine2);

        BlitStringToBuffer(BattleTextLine2Scratch, &g6x7Font, &TextColor, 67, 139);
    }

    // Don't start drawing line 3 until line 2 is finished animating.
    if (strlen(BattleTextLine2Scratch) == strlen(gBattleTextLine2))
    {
        // Old fashioned typewriter animation for the text.
        if (LocalFrameCounter % 3 == 0)
        {
            if (BattleTextLine3CharactersToShow <= strlen(gBattleTextLine3))
            {
                BattleTextLine3CharactersToShow++;
            }
        }

        snprintf(BattleTextLine3Scratch, BattleTextLine3CharactersToShow, "%s", gBattleTextLine3);

        BlitStringToBuffer(BattleTextLine3Scratch, &g6x7Font, &TextColor, 67, 147);
    }





    DrawPlayerStatsWindow(&TextColor);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}