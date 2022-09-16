#include "Main.h"

#include "Battle.h"

const MONSTER gSlime001 = {
    .Name = "Slime",
    .Sprite = &gMonsterSprite_Slime_001,
    .MaxHP = 5,
    .HP = 5,
    .MaxMP = 0,
    .MP = 0,
    .XP = 5,
    .Money = 2,
    .Damage = 1,
    .Dexterity = 1,
    .Strength = 1,
    .Luck = 1,
    .Intelligence = 1,
    .Evasion = 0,
    .Defense = 0,
    .Emotes = { "*jiggles menacingly*", "*spits and gurgles*", "*cold, dead-eyed stare*" },
    .KnownSpells = { 0 },
    .AttackChance = 95,
    .RunChance = 5,
    .SpellChance = 0,
    .DefendChance = 0,
    .BaseDamage = 1
};

const MONSTER gRat001 = {
    .Name = "Rat",
    .Sprite = &gMonsterSprite_Rat_001,
    .MaxHP = 5,
    .HP = 5,
    .MaxMP = 0,
    .MP = 0,
    .XP = 5,
    .Money = 2,
    .Damage = 1,
    .Dexterity = 2,
    .Strength = 1,
    .Luck = 1,
    .Intelligence = 1,
    .Evasion = 1,
    .Defense = 0,
    .Emotes = { "*squeak squeak*", "*whiskers twitching angrily*", "*looks like it might have rabies*" },
    .KnownSpells = { 0 },
    .AttackChance = 95,
    .RunChance = 5,
    .SpellChance = 0,
    .DefendChance = 0,
    .BaseDamage = 1
};

// This holds a copy of one of the monster templates.
MONSTER gCurrentMonster = { 0 };

const MONSTER* gEasyOutdoorMonsters[] = { &gSlime001, &gRat001 };

const MONSTER* gMediumOutdoorMonsters[] = { 0 };

const MONSTER* gHardOutdoorMonsters[] = { 0 };

// A random string like "A wild %s draws near!"
char gBattleTextLine1[64];

// The second line of text, whether this is a surprise attack or not.
char gBattleTextLine2[64];

// Third line of battle text, this is a randomly selected monster emote.
char gBattleTextLine3[64];

// Is a "surprise attack" if this is set to true during random monster generation.
BOOL gSurpriseAttack;

// Did the attack land?
BOOL gHit;

// If so, how much damage did it do?
int gDamageDealt;

// Was it a critical hit?
BOOL gCritical;

BOOL gWaitOnDialog;

// When the monster dies
int gMonsterFade;

BATTLESTATE gBattleState;

BATTLESTATE gPreviousBattleState;

// Use the frame counter to cause the monster to shake when we hit it.
uint64_t gMonsterShake;

// When the monster hits the player, the UI will shake
uint64_t gWindowShake;



MENUITEM gMI_BattleAction_Attack = { "Attack", 142, 200, TRUE, MenuItem_BattleAction_Attack };

MENUITEM gMI_BattleAction_Spell  = { "Spell",  142, 210, TRUE, MenuItem_BattleAction_Spell };

MENUITEM gMI_BattleAction_Defend = { "Defend", 208, 200, TRUE, MenuItem_BattleAction_Defend };

MENUITEM gMI_BattleAction_Item   = { "Item",   208, 210, TRUE, MenuItem_BattleAction_Item };

MENUITEM gMI_BattleAction_Run    = { "Run",    208, 220, TRUE, MenuItem_BattleAction_Run };

MENUITEM* gMI_PlayerBattleActionItems[] = { 
    &gMI_BattleAction_Attack, 
    &gMI_BattleAction_Spell,    
    &gMI_BattleAction_Defend,
    &gMI_BattleAction_Item,
    &gMI_BattleAction_Run };

MENU gMenu_PlayerBattleAction = { "Your action:", 0, _countof(gMI_PlayerBattleActionItems), gMI_PlayerBattleActionItems };






void GenerateMonster(void)
{
    unsigned int RandomValue = 0;

    rand_s(&RandomValue);

    // Make a copy of the monster for the player to fight; don't modify the template monster.
    memcpy(&gCurrentMonster, gEasyOutdoorMonsters[RandomValue % _countof(gEasyOutdoorMonsters)], sizeof(MONSTER));
    
    ASSERT(
        gCurrentMonster.AttackChance +
        gCurrentMonster.DefendChance +
        gCurrentMonster.SpellChance +
        gCurrentMonster.RunChance == 100, "These four need to add up to 100!");

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


    // Re-roll another random value to determine whether the monster gets to attack first or not.
    rand_s(&RandomValue);

    if (RandomValue % 5 == 0)
    {
        gSurpriseAttack = TRUE;        

        rand_s(&RandomValue);

        if (RandomValue % 2 == 0)
        {
            sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You are caught off guard!");
        }
        else
        {
            sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "%s snuck up behind you!", gCurrentMonster.Name);
        }
    }
    else
    {
        gSurpriseAttack = FALSE;

        rand_s(&RandomValue);

        if (RandomValue % 2 == 0)
        {
            sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You are ready for battle.");
        }
        else
        {
            sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You assume fighting stance, ready to act.");
        }
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
    // The player can select menu items only if it is the player's turn, and only if the dialog is finished.
    if ((gBattleState == BATTLESTATE_PLAYERTHINKING) && !gWaitOnDialog && (gPlayer.HP > 0))
    {
        if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
        {
            switch (gMenu_PlayerBattleAction.SelectedItem)
            {
                case 0:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem = 1;

                    break;
                }
                case 1:
                case 3:
                case 4:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem--;

                    break;
                }
                case 2:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem = 4;

                    break;
                }
                default:
                {
                    ASSERT(FALSE, "Wrong number of menu items?")
                }
            }
        }
        else if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
        {
            switch (gMenu_PlayerBattleAction.SelectedItem)
            {
                case 0:
                case 2:
                case 3:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem++;

                    break;
                }
                case 1:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem = 0;

                    break;
                }
                case 4:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem = 2;

                    break;
                }
                default:
                {
                    ASSERT(FALSE, "Wrong number of menu items?");
                }
            }
        }
        else if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
        {
            switch (gMenu_PlayerBattleAction.SelectedItem)
            {
                case 0:
                case 1:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem += 2;

                    break;
                }
                case 2:
                case 3:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem -= 2;

                    break;
                }
                case 4:
                {
                    break;
                }
                default:
                {
                    ASSERT(FALSE, "Wrong number of menu items?");
                }
            }
        }
        else if (gGameInput.RightKeyIsDown && !gGameInput.RightKeyWasDown)
        {
            switch (gMenu_PlayerBattleAction.SelectedItem)
            {
                case 0:
                case 1:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem += 2;

                    break;
                }
                case 2:
                case 3:
                {
                    PlayGameSound(&gSoundMenuNavigate);

                    gMenu_PlayerBattleAction.SelectedItem -= 2;

                    break;
                }
                case 4:
                {

                    break;
                }
                default:
                {
                    ASSERT(FALSE, "Wrong number of menu items?");
                }
            }
        }
        else if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
        {
            gMenu_PlayerBattleAction.Items[gMenu_PlayerBattleAction.SelectedItem]->Action();
        }    
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        if (!gWaitOnDialog)
        {
            unsigned int RandomValue = 0;

            if (gBattleState == BATTLESTATE_PLAYERISDEAD)
            {
                ResetEverythingForNewGame();

                return;
            }

            if (gPlayer.HP <= 0)
            {
                gBattleState = BATTLESTATE_PLAYERISDEAD;
            }
            else
            {
                switch (gBattleState)
                {
                    case BATTLESTATE_PLAYERATTACKING:
                    {
                        if (gCurrentMonster.HP > 0)
                        {
                            gBattleState = BATTLESTATE_MONSTERTHINKING;
                        }
                        else
                        {
                            gBattleState = BATTLESTATE_MONSTERISDEAD;
                        }

                        break;
                    }
                    case BATTLESTATE_MONSTERISDEAD:
                    {
                        gPreviousGameState = gCurrentGameState;

                        gCurrentGameState = GAMESTATE_OVERWORLD;

                        LogMessageA(LL_DEBUG, "[%s] Transitioning from game state %d to %d. Player killed a monster.",
                            __FUNCTION__,
                            gPreviousGameState,
                            gCurrentGameState);

                        StopMusic();

                        break;
                    }
                    case BATTLESTATE_MONSTERTHINKING:
                    {
                        // Here the monster needs to decide what it wants to do!

                        rand_s(&RandomValue);

                        RandomValue %= 101;

                        if (gCurrentMonster.HP < (gCurrentMonster.MaxHP * 0.2f))
                        {
                            gCurrentMonster.RunChance += 10;

                            gCurrentMonster.AttackChance -= 10;
                        }

                        if (gCurrentMonster.AttackChance >= (int)RandomValue)
                        {
                            MonsterAttack();
                        }
                        else if (gCurrentMonster.DefendChance >= (int)RandomValue)
                        {
                            gBattleState = BATTLESTATE_MONSTERDEFENDING;
                        }
                        else if (gCurrentMonster.SpellChance >= (int)RandomValue)
                        {
                            BOOL CanCastSpell = FALSE;

                            for (int spell = 0; spell < _countof(gCurrentMonster.KnownSpells) - 1; spell++)
                            {
                                if (gCurrentMonster.KnownSpells[spell].Cost <= gCurrentMonster.MP)
                                {
                                    CanCastSpell = TRUE;
                                }
                            }

                            if (CanCastSpell)
                            {
                                gBattleState = BATTLESTATE_MONSTERCASTINGSPELL;
                            }
                            else
                            {
                                if (gCurrentMonster.RunChance >= (int)RandomValue)
                                {
                                    gBattleState = BATTLESTATE_MONSTERRUNNINGAWAY;
                                }
                                else
                                {
                                    MonsterAttack();
                                }
                            }
                        }
                        else if (gCurrentMonster.RunChance >= (int)RandomValue)
                        {
                            gBattleState = BATTLESTATE_MONSTERRUNNINGAWAY;
                        }
                        else
                        {
                            // e.g. this might happen if we roll 100, but the monster does not have
                            // a 100% chance of doing anything. Let's default to... 

                            MonsterAttack();
                        }

                        break;
                    }
                }
            }
        }
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

    static unsigned int BattleTextLine1CharactersToShow;

    static unsigned int BattleTextLine2CharactersToShow;

    static unsigned int BattleTextLine3CharactersToShow;

    char BattleTextLine1Scratch[64] = { 0 };

    char BattleTextLine2Scratch[64] = { 0 };

    char BattleTextLine3Scratch[64] = { 0 };

    static uint64_t MonsterShakeDiff;

    static uint64_t WindowShakeDiff;

    static int WindowShakeX;

    static int WindowShakeY;

    static int MonsterDeadFadeOut;

    static uint64_t MonsterDeathFrame;

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

        gWaitOnDialog = TRUE;

        BattleTextLine1CharactersToShow = 0;

        BattleTextLine2CharactersToShow = 0;

        BattleTextLine3CharactersToShow = 0;       

        gMenu_PlayerBattleAction.SelectedItem = 0;

        gBattleState = BATTLESTATE_INTRO;

        gPreviousBattleState = BATTLESTATE_INTRO;

        gCritical = FALSE;

        gMonsterShake = 0;

        gWindowShake = 0;        

        MonsterDeadFadeOut = 0;

        MonsterDeathFrame = 0;
    }

#ifdef SMOOTH_FADES
    // Here is a smoother fade in that looks nicer, but the original NES was not capable of such smooth gradients and fade
    // effects. We will have to decide which we prefer later - looks better, or is more faithful to the original hardware?

    if (AlphaAdjust < 0)
    {
        AlphaAdjust += 4;
    }

    if (gBattleState == BATTLESTATE_MONSTERISDEAD)
    {
        if (MonsterDeadFadeOut > -255)
        {
            MonsterDeadFadeOut -= 4;
        }
    }
#else
    // Here is an easy, "chunky" fade-in from black in 4 steps, that sort of has a similar feel
    // to the kind of fade-in you might have seen on the classic NES. AlphaAdjust starts at -256 and ends at 0.
    switch (LocalFrameCounter)
    {
        case 15:
        case 30:
        case 45:
        case 60:
        {
            AlphaAdjust += 64;
        }
    }

    if (gBattleState == BATTLESTATE_MONSTERISDEAD)
    {
        uint64_t FrameDifference = LocalFrameCounter - MonsterDeathFrame;

        switch (FrameDifference)
        {
            case 15:
            case 30:
            case 45:
            case 60:
            {
                MonsterDeadFadeOut -= 64;
            }
        }
    }
#endif
    
    // It doesn't feel very nice to have to wait the full 60 frames for the fade-in to complete in order for 
    // input to be enabled again. We should enable it sooner so the kids with fast reflexes can work the menus quickly.
    if (LocalFrameCounter == REENABLE_INPUT_AFTER_X_FRAMES_DELAY)
    {
        gInputEnabled = TRUE;
    }

    if (LocalFrameCounter == 0)
    {
        StopMusic();

        PlayGameMusic(&gMusicBattleIntro01, FALSE, TRUE);

        PlayGameMusic(&gMusicBattle01, TRUE, FALSE);
        
        GenerateMonster();

        switch (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16])
        {        
            case TILE_SAND_01:
            case TILE_SWAMP_01:
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

    // Either the player or the monster has taken an action.
    if (gBattleState != gPreviousBattleState)
    {
        unsigned int RandomValue = 0; 

        gWaitOnDialog = TRUE; 

        BattleTextLine1CharactersToShow = 0;
        
        BattleTextLine2CharactersToShow = 0;
        
        BattleTextLine3CharactersToShow = 0;

        switch (gBattleState)
        {            
            case BATTLESTATE_PLAYERATTACKING:
            {
                sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "You attack %s with your %s!", gCurrentMonster.Name, gPlayer.Inventory[EQUIPPED_WEAPON].Name);
                        
                if (gHit)
                {   
                    PlayGameSound(&gSoundHit01);

                    gMonsterShake = LocalFrameCounter;

                    if (gCritical)
                    {
                        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "Critical hit on %s for %d damage!", gCurrentMonster.Name, gDamageDealt);
                    }
                    else
                    {
                        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You hit the %s for %d damage!", gCurrentMonster.Name, gDamageDealt);
                    }   
                }        
                else        
                {
                    PlayGameSound(&gSoundMiss01);

                    sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You missed!");        
                }

                if (gCurrentMonster.HP < (gCurrentMonster.MaxHP * 0.2f))
                {
                    sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "%s looks like it's about to die.", gCurrentMonster.Name);
                }
                else
                {
                    // Re-roll to select a random monster emote.
                    rand_s(&RandomValue);

                    sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "%s: %s", gCurrentMonster.Name, gCurrentMonster.Emotes[(RandomValue % 3)]);
                }

                break;
            }
            case BATTLESTATE_PLAYERTHINKING:
            {
                sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "You plan your next move against %s...", gCurrentMonster.Name);

                sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "...");

                sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "...");

                break;
            }
            case BATTLESTATE_MONSTERTHINKING:
            {
                rand_s(&RandomValue);

                if (RandomValue % 2 == 0)
                {
                    sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "%s is moving...", gCurrentMonster.Name);
                }
                else
                {
                    sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "%s changes its stance...", gCurrentMonster.Name);
                }

                rand_s(&RandomValue);

                if (RandomValue % 2 == 0)
                {
                    sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "%s looks like it's preparing to attack...", gCurrentMonster.Name);
                }
                else
                {
                    sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You're unsure of what %s is thinking...", gCurrentMonster.Name);
                }

                rand_s(&RandomValue);

                if (RandomValue % 2 == 0)
                {
                    sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "You prepare yourself as %s lunges!", gCurrentMonster.Name);
                }
                else
                {
                    sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "%s moves quickly!", gCurrentMonster.Name);
                }

                break;
            }
            case BATTLESTATE_MONSTERISDEAD:
            {
                MonsterDeathFrame = LocalFrameCounter;

                gPlayer.Money += gCurrentMonster.Money;

                gPlayer.XP += gCurrentMonster.XP;

                StopMusic();

                PlayGameMusic(&gMusicVictoryIntro, FALSE, TRUE);

                PlayGameMusic(&gMusicVictoryLoop, TRUE, FALSE);

                rand_s(&RandomValue);

                if (RandomValue % 3 == 0)
                {
                    sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "%s is dead!", gCurrentMonster.Name);
                }
                else if (RandomValue % 2 == 0)
                {
                    sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "%s has been vanquished!", gCurrentMonster.Name);
                }
                else
                {
                    sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "You have defeated %s!", gCurrentMonster.Name);
                }

                sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You gain %d experience and %d gold.", gCurrentMonster.XP, gCurrentMonster.Money);

                sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "");

                break;
            }
            case BATTLESTATE_MONSTERATTACKING:
            {
                sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "%s attacks you!", gCurrentMonster.Name);
                
                if (gHit)
                {
                    PlayGameSound(&gSoundHit01);

                    gWindowShake = LocalFrameCounter;

                    if (gCritical)
                    {
                        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "%s critically hits you for %d damage!", gCurrentMonster.Name, gDamageDealt);
                    }
                    else
                    {
                        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "%s hits you for %d damage!", gCurrentMonster.Name, gDamageDealt);
                    }
                }
                else
                {
                    PlayGameSound(&gSoundMiss01);

                    rand_s(&RandomValue);

                    if (RandomValue % 2 == 0)
                    {
                        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "%s misses you.", gCurrentMonster.Name);
                    }
                    else
                    {
                        sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "You evade %s's attack.", gCurrentMonster.Name);
                    }
                }

                if (gPlayer.HP > (gPlayer.MaxHP * 0.20f))
                {
                    rand_s(&RandomValue);

                    if (RandomValue % 3 == 0)
                    {
                        sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "Adrenaline surges through your veins.");
                    }
                    else if (RandomValue % 2 == 0)
                    {
                        sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "You don't have time to bleed.");
                    }
                    else
                    {
                        sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "You focus and plan your next move.");
                    }
                }
                else
                {
                    rand_s(&RandomValue);

                    if (RandomValue % 2 == 0)
                    {
                        sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "You feel like you are about to die.");
                    }
                    else
                    {
                        sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "You cough up blood. You're not feeling well.");
                    }
                }

                break;
            }
            case BATTLESTATE_PLAYERISDEAD:
            {
                StopMusic();

                sprintf_s(gBattleTextLine1, sizeof(gBattleTextLine1), "You are dead.");

                sprintf_s(gBattleTextLine2, sizeof(gBattleTextLine2), "The world needed a hero.");

                sprintf_s(gBattleTextLine3, sizeof(gBattleTextLine3), "You were not it.");

                break;
            }
            default:
            {
                ASSERT(FALSE, "BattleState not implemented!");
            }
        }
    }

    if (gWindowShake)
    {
        WindowShakeDiff = LocalFrameCounter - gWindowShake;

        switch (WindowShakeDiff)
        {
            case 0:
            {
                WindowShakeX = 1;

                WindowShakeY = 0;

                break;
            }
            case 6:
            {
                WindowShakeX = 0;

                WindowShakeY = -1;

                break;
            }
            case 12:
            {
                WindowShakeX = -1;

                WindowShakeY = 0;

                break;
            }
            case 18:
            {
                WindowShakeX = 0;

                WindowShakeY = 1;

                break;
            }
            case 24:
            {
                WindowShakeX = 0;

                WindowShakeY = 0;

                WindowShakeDiff = 0;

                gWindowShake = 0;

                break;
            }
        }
    }
    
    BlitBackground(
        &gOverworld01.GameBitmap,        
        AlphaAdjust);

    DrawPlayerStatsWindow(AlphaAdjust, WindowShakeX, WindowShakeY);

    // Draw the border around the monster battle scene.
    DrawWindow(
        (GAME_RES_WIDTH / 2) - (100 / 2) + WindowShakeX,
        14 + WindowShakeY,
        100, 
        100,
        (gPlayer.HP <= (gPlayer.MaxHP * 0.20f)) ?
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0x00, .Colors.Blue = 0x00 } :
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0xFC, .Colors.Blue = 0xFC },
        NULL,
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0x40, .Colors.Green = 0x40, .Colors.Blue = 0x40 },
        WINDOW_FLAG_BORDERED | WINDOW_FLAG_THICK | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_SHADOW);

    // Draw the battle scene, aka the backdrop behind the monster.
    if (BattleScene != 0)
    {
        Blit32BppBitmapEx(
            BattleScene,
            ((GAME_RES_WIDTH / 2) - (BattleScene->BitmapInfo.bmiHeader.biWidth / 2)) + WindowShakeX, 
            16 + WindowShakeY, 
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
        static int MonsterShakeX;

        static int MonsterShakeY;

        if (gMonsterShake)
        {
            MonsterShakeDiff = LocalFrameCounter - gMonsterShake;

            switch (MonsterShakeDiff)
            {
                case 0:
                {
                    MonsterShakeX = 1;

                    MonsterShakeY = 0;

                    break;
                }
                case 6:
                {
                    MonsterShakeX = 0;

                    MonsterShakeY = -1;

                    break;
                }
                case 12:
                {
                    MonsterShakeX = -1;

                    MonsterShakeY = 0;

                    break;
                }
                case 18:
                {
                    MonsterShakeX = 0;

                    MonsterShakeY = 1;

                    break;
                }
                case 24:
                {
                    MonsterShakeX = 0;

                    MonsterShakeY = 0;

                    MonsterShakeDiff = 0;

                    gMonsterShake = 0;

                    break;
                }
            }
        }

        Blit32BppBitmapEx(
            gCurrentMonster.Sprite, 
            ((GAME_RES_WIDTH / 2) - (gCurrentMonster.Sprite->BitmapInfo.bmiHeader.biWidth / 2)) + MonsterShakeX,
            48 + MonsterShakeY,
            0 + (int)MonsterShakeDiff,
            0 + (int)MonsterShakeDiff,
            0 + ((int)MonsterShakeDiff * 3),
            (gBattleState == BATTLESTATE_MONSTERISDEAD) ? MonsterDeadFadeOut : AlphaAdjust,
            BLIT_FLAG_ALPHABLEND);
    } 
    else
    {
        ASSERT(FALSE, "Monster bitmap is NULL!");
    }
    
    // Draw the window where the battle text will go.
    DrawWindow(
        (GAME_RES_WIDTH / 2) - (300 / 2) + WindowShakeX,
        128 + WindowShakeY, 
        300, 
        96,
        (gPlayer.HP <= (gPlayer.MaxHP * 0.20f)) ?
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0x00, .Colors.Blue = 0x00 } :
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0xFC, .Colors.Blue = 0xFC },
        gWindowShake ?
        &(PIXEL32) { .Colors.Red = 0xFF, .Colors.Green = 0x00, .Colors.Blue = 0x00, .Colors.Alpha = 0xFF } :
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0x00, .Colors.Green = 0x00, .Colors.Blue = 0x00 },
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0x40, .Colors.Green = 0x40, .Colors.Blue = 0x40 },
        WINDOW_FLAG_ALPHABLEND | WINDOW_FLAG_OPAQUE | WINDOW_FLAG_BORDERED | WINDOW_FLAG_THICK | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_SHADOW);

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
        45, 
        132,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
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
            45, 
            141,
            (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
            (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
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
            45, 
            150,
            (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
            (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
            255,
            AlphaAdjust,
            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
    }

    // Once the third line of text's animation is complete, we are ready to start fighting.
    if (strlen(BattleTextLine3Scratch) == strlen(gBattleTextLine3))
    {
        gWaitOnDialog = FALSE;

        switch (gBattleState)
        {
            case BATTLESTATE_INTRO:
            {
                if (gSurpriseAttack)
                {
                    gBattleState = BATTLESTATE_MONSTERTHINKING;
                }
                else
                {
                    gBattleState = BATTLESTATE_PLAYERTHINKING;
                }

                break;
            }
            case BATTLESTATE_MONSTERATTACKING:
            case BATTLESTATE_MONSTERCASTINGSPELL:
            {
                gBattleState = BATTLESTATE_PLAYERTHINKING;                

                break;
            }
        }        

        if (gBattleState == BATTLESTATE_PLAYERTHINKING && (gPlayer.HP > 0))
        {            
            BlitStringEx(
                gMenu_PlayerBattleAction.Name,
                &g6x7Font,
                67,
                175,
                (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
                (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
        
            // Draw the window where the player's choices will go        
            DrawWindow(
                0,
                195,
                120,
                40,
                (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 
                &(PIXEL32) { .Colors.Alpha = 0xFF, .Colors.Red = 0xFC, .Colors.Green = 0x00, .Colors.Blue = 0x00 } :
                &(PIXEL32) { .Colors.Alpha = 0xFF, .Colors.Red = 0xFC, .Colors.Green = 0xFC, .Colors.Blue = 0xFC },
                &COLOR_NES_BLACK,
                NULL,
                WINDOW_FLAG_BORDERED | WINDOW_FLAG_THICK | WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_OPAQUE | WINDOW_FLAG_ROUNDED_CORNERS);

            for (int Counter = 0; Counter < gMenu_PlayerBattleAction.ItemCount; Counter++)
            {
                BlitStringEx(
                    gMenu_PlayerBattleAction.Items[Counter]->Name,
                    &g6x7Font,
                    gMenu_PlayerBattleAction.Items[Counter]->x,
                    gMenu_PlayerBattleAction.Items[Counter]->y,
                    (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
                    (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
                    255,
                    AlphaAdjust,
                    BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
            }                       
            
            BlitStringEx(
                "\xBB",
                &g6x7Font,
                gMenu_PlayerBattleAction.Items[gMenu_PlayerBattleAction.SelectedItem]->x - 6,
                gMenu_PlayerBattleAction.Items[gMenu_PlayerBattleAction.SelectedItem]->y,
                (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
                (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
        }        
    }    

    gPreviousBattleState = gBattleState;

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}


void MenuItem_BattleAction_Attack(void)
{
    // Determine hit/miss 
    // Determine damage dealt    

    unsigned int LuckFactor = 0;
    
    gBattleState = BATTLESTATE_PLAYERATTACKING;

    gWaitOnDialog = TRUE;
    
    // First decide whether we landed the blow.    

    rand_s(&LuckFactor);

    if ((gPlayer.Dexterity + (LuckFactor % (gPlayer.Luck + 1)) >= gCurrentMonster.Evasion))
    {
        gHit = TRUE;
    }
    else
    {
        gHit = FALSE;
    }

    if (gHit)
    {
        // Calculate damage dealt.

        gDamageDealt = (gPlayer.Strength + gPlayer.Inventory[EQUIPPED_WEAPON].Damage) - gCurrentMonster.Defense;

        rand_s(&LuckFactor);

        LuckFactor %= 100;

        if ((gPlayer.Luck + gPlayer.Dexterity) > LuckFactor)
        {
            gCritical = TRUE;
        }
        else
        {
            gCritical = FALSE;
        }

        // A critical hit is basically a free second hit.
        if (gCritical)
        {
            gDamageDealt += (gPlayer.Strength + gPlayer.Inventory[EQUIPPED_WEAPON].Damage) - gCurrentMonster.Defense;
        }

        gCurrentMonster.HP -= gDamageDealt;
    }
    else
    {
        gDamageDealt = 0;
    }    
}

void MenuItem_BattleAction_Spell(void)
{

}

void MenuItem_BattleAction_Run(void)
{

}

void MenuItem_BattleAction_Defend(void)
{

}

void MenuItem_BattleAction_Item(void)
{

}

void MonsterAttack(void)
{
    // Determine hit/miss 
    // Determine damage dealt    

    unsigned int LuckFactor = 0;

    gBattleState = BATTLESTATE_MONSTERATTACKING;

    gWaitOnDialog = TRUE;

    // First decide whether we landed the blow.    

    rand_s(&LuckFactor);

    if ((gCurrentMonster.Dexterity + (LuckFactor % (gCurrentMonster.Luck + 1)) >= gPlayer.Evasion))
    {
        gHit = TRUE;
    }
    else
    {
        gHit = FALSE;
    }

    if (gHit)
    {
        // Calculate damage dealt.

        gDamageDealt = (gCurrentMonster.Strength + gCurrentMonster.BaseDamage) - gPlayer.Defense;

        rand_s(&LuckFactor);

        LuckFactor %= 100;

        if ((gCurrentMonster.Luck + gCurrentMonster.Dexterity) > LuckFactor)
        {
            gCritical = TRUE;
        }
        else
        {
            gCritical = FALSE;
        }

        // A critical hit is basically a free second hit.
        if (gCritical)
        {
            gDamageDealt += (gCurrentMonster.Strength + gCurrentMonster.BaseDamage) - gPlayer.Defense;
        }

        gPlayer.HP -= gDamageDealt;

        if (gPlayer.HP < 0)
        {
            gPlayer.HP = 0;
        }
    }
    else
    {
        gDamageDealt = 0;
    }
}