#include "Main.h"

#include "Battle.h"

// This holds a copy of one of the monster templates.
MONSTER gCurrentMonster = { 0 };

const MONSTER gSlime001 = {
    .Name = "Slime",
    .Sprite = &gMonsterSprite_Slime_001,
    .BaseHP = 5,
    .BaseMP = 0,
    .BaseXP = 10,
    .BaseGP = 2,
    .BaseDamage = 1,
    .RegularAttackChance = 99,
    .RegularCastSpellChance = 0,
    .RegularRunChance = 1,
    .LowHPAttackChance = 80,
    .LowHPCastSpellChance = 0,
    .LowHPRunChance = 20,
    .LowHPThresholdPercent = 10,
    .CriticalChance = 5,
    .CriticalMultiplier = 2.0f,
    .DodgeChance = 5,
    .Armor = 0,
    .RunSuccessChance = 25,
    .Emotes = { "*jiggles menacingly*", "*spits and gurgles*", "*cold, dead-eyed stare*" },
    .KnownSpells = { 0 }
};

const MONSTER gRat001 = { 
    .Name = "Rat", 
    .Sprite = &gMonsterSprite_Rat_001, 
    .BaseHP = 10, 
    .BaseMP = 0, 
    .BaseXP = 15,
    .BaseGP = 1,
    .BaseDamage = 1,
    .RegularAttackChance = 99,
    .RegularCastSpellChance = 0,
    .RegularRunChance = 1,
    .LowHPAttackChance = 80,
    .LowHPCastSpellChance = 0,
    .LowHPRunChance = 20,
    .LowHPThresholdPercent = 10,
    .CriticalChance = 5,
    .CriticalMultiplier = 2.0f,
    .DodgeChance = 5,
    .Armor = 0,
    .RunSuccessChance = 50,
    .Emotes = { "*squeak squeak*", "*whiskers twitching angrily*", "*looks like it might have rabies*" },
    .KnownSpells = { 0 } 
};

const MONSTER* gOutdoorMonsters[] = { &gSlime001, &gRat001 };

// A random string like "A wild %s draws near!"
char gBattleTextLine1[64];

// The second line of text, whether this is a surprise attack or not.
char gBattleTextLine2[64];

// Third line of battle text, this is a randomly selected monster emote.
char gBattleTextLine3[64];

// Is a "surprise attack" if this is set to true during random monster generation.

BOOL gMonstersTurn;

MENUITEM gMI_PlayerBattleAction_Attack = { "Attack", (GAME_RES_WIDTH / 2) - ((6 * 6) / 2), 191, TRUE, MenuItem_PlayerBattleAction_Attack };

MENUITEM gMI_PlayerBattleAction_Spell = { "Spell", 143, 200, TRUE, MenuItem_PlayerBattleAction_Spell };

MENUITEM gMI_PlayerBattleAction_Run = { "Run", 232, 200, TRUE, MenuItem_PlayerBattleAction_Run };

MENUITEM gMI_PlayerBattleAction_Wait = { "Wait", (GAME_RES_WIDTH / 2) - ((6 * 4) / 2), 210, TRUE, MenuItem_PlayerBattleAction_Wait};

MENUITEM* gMI_PlayerBattleActionItems[] = { 
    &gMI_PlayerBattleAction_Attack, 
    &gMI_PlayerBattleAction_Spell,
    &gMI_PlayerBattleAction_Run,
    &gMI_PlayerBattleAction_Wait };

MENU gMenu_PlayerBattleAction = { "Your action:", 0, _countof(gMI_PlayerBattleActionItems), gMI_PlayerBattleActionItems };

BOOL gWaitingForPlayerInput;

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
        gMonstersTurn = TRUE;

        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You are caught off guard!");
    }
    else
    {
        gMonstersTurn = FALSE;

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
    if (gWaitingForPlayerInput)
    {
        if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
        {
            gMenu_PlayerBattleAction.SelectedItem = 1;
        }
        else if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
        {
            gMenu_PlayerBattleAction.SelectedItem = 0;
        }
        else if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
        {
            gMenu_PlayerBattleAction.SelectedItem = 3;
        }
        else if (gGameInput.RightKeyIsDown && !gGameInput.RightKeyWasDown)
        {
            gMenu_PlayerBattleAction.SelectedItem = 2;
        }
    }

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

    static int AlphaAdjust = -256;

    static GAMEBITMAP* BattleScene = NULL;

    // These strings are "scratch space" to display only portions
    // of the full line of text. This is used to make a "typewriter" like animation.

    static unsigned int BattleTextLine1CharactersToShow = 0;

    static unsigned int BattleTextLine2CharactersToShow = 0;

    static unsigned int BattleTextLine3CharactersToShow = 0;

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

        AlphaAdjust = -256;

        gInputEnabled = FALSE;

        BattleTextLine1CharactersToShow = 0;

        BattleTextLine2CharactersToShow = 0;

        BattleTextLine3CharactersToShow = 0;

        gWaitingForPlayerInput = FALSE;
    }

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
    
    BlitBackgroundEx(
        &gOverworld01.GameBitmap,
        0,
        0,
        0,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND);

    // Draw the border around the monster battle scene.
    DrawWindow(0, 14, 100, 100,
        &COLOR_NES_WHITE,
        &COLOR_NES_BLACK,
        &COLOR_NES_BLACK,
        WINDOW_FLAG_OPAQUE | WINDOW_FLAG_BORDERED | WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_THICK | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_SHADOW);

    // Draw the battle scene, aka the backdrop behind the monster.
    if (BattleScene != 0)
    {
        Blit32BppBitmapToBufferEx(
            BattleScene, 
            ((GAME_RES_WIDTH / 2) - (BattleScene->BitmapInfo.bmiHeader.biWidth / 2)), 
            16, 
            0,
            0,
            0,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND);
    }
    else
    {
        ASSERT(FALSE, "BattleScene is NULL!");
    }

    // Draw the monster.
    if (gCurrentMonster.Sprite->Memory != NULL)
    {
        Blit32BppBitmapToBufferEx(
            gCurrentMonster.Sprite, 
            ((GAME_RES_WIDTH / 2) - (gCurrentMonster.Sprite->BitmapInfo.bmiHeader.biWidth / 2)), 
            48, 
            0,
            0,
            0,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND);
    } 
    
    // Draw the window where the battle text will go.
    DrawWindow(0, 128, 256, 96,
        &COLOR_NES_WHITE,
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

    BlitStringEx(
        BattleTextLine1Scratch, 
        &g6x7Font,        
        67, 
        132,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

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

        BlitStringEx(
            BattleTextLine2Scratch, 
            &g6x7Font, 
            67, 
            141,
            255,
            255,
            255,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
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

        BlitStringEx(
            BattleTextLine3Scratch, 
            &g6x7Font,
            67, 
            150,
            255,
            255,
            255,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
    }

    // Once the third line of text's animation is complete, we are ready to start fighting.
    if (strlen(BattleTextLine3Scratch) == strlen(gBattleTextLine3))
    {
        if (gMonstersTurn)
        {
            // Monster chooses an action
            // Monster can try to hit player, cast a spell on player, or run away from player.

        }
        else
        {
            // Player chooses an action
            // Player can try to hit monster, cast a spell on monster, or run away from monster.

            gWaitingForPlayerInput = TRUE;

            BlitStringEx(
                gMenu_PlayerBattleAction.Name, 
                &g6x7Font, 
                67, 
                175,
                255,
                255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

            DrawWindow(0, 188, 120, 32, &COLOR_NES_WHITE, &COLOR_NES_BLACK, NULL, 
                WINDOW_FLAG_BORDERED | WINDOW_FLAG_THICK | WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_OPAQUE | WINDOW_FLAG_ROUNDED_CORNERS);

            for (int Counter = 0; Counter < gMenu_PlayerBattleAction.ItemCount; Counter++)
            {
                BlitStringEx(
                    gMenu_PlayerBattleAction.Items[Counter]->Name,
                    &g6x7Font,                    
                    gMenu_PlayerBattleAction.Items[Counter]->x,
                    gMenu_PlayerBattleAction.Items[Counter]->y,
                    255,
                    255,
                    255,
                    AlphaAdjust,
                    BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
            }

            BlitStringEx(
                "\xBB",
                &g6x7Font,                
                gMenu_PlayerBattleAction.Items[gMenu_PlayerBattleAction.SelectedItem]->x - 6,
                gMenu_PlayerBattleAction.Items[gMenu_PlayerBattleAction.SelectedItem]->y,
                255,
                255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
        }
    }

    //DrawPlayerStatsWindow(&TextColor);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void MenuItem_PlayerBattleAction_Attack(void)
{

}

void MenuItem_PlayerBattleAction_Spell(void)
{

}

void MenuItem_PlayerBattleAction_Run(void)
{

}

void MenuItem_PlayerBattleAction_Wait(void)
{

}