// Filename: CharacterNamingScreen.c
// Contains code for the functions that are specific to the character naming screen.
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

#include "CharacterNamingScreen.h"

// The character naming screen looks like this:
// A B C D E F G H I J K L M 
// N O P Q R S T U V W X Y Z
// a b c d e f g h i j k l m
// n o p q r s t u v w x y z
// Back                   OK
								 // (12 * 8) + (13 * 6) 12 spaces at 8 pixels each, plus 13 letters at 6 pixels wide each
MENUITEM gMI_CharacterNaming_A = { "A", 118, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_B = { "B", 130, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_C = { "C", 142, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_D = { "D", 154, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_E = { "E", 166, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_F = { "F", 178, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_G = { "G", 190, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_H = { "H", 202, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_I = { "I", 214, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_J = { "J", 226, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_K = { "K", 238, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_L = { "L", 250, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_M = { "M", 262, 110, TRUE, MenuItem_CharacterNaming_Add };



MENUITEM gMI_CharacterNaming_N = { "N", 118, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_O = { "O", 130, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_P = { "P", 142, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_Q = { "Q", 154, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_R = { "R", 166, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_S = { "S", 178, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_T = { "T", 190, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_U = { "U", 202, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_V = { "V", 214, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_W = { "W", 226, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_X = { "X", 238, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_Y = { "Y", 250, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_Z = { "Z", 262, 120, TRUE, MenuItem_CharacterNaming_Add };


MENUITEM gMI_CharacterNaming_a = { "a", 118, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_b = { "b", 130, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_c = { "c", 142, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_d = { "d", 154, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_e = { "e", 166, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_f = { "f", 178, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_g = { "g", 190, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_h = { "h", 202, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_i = { "i", 214, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_j = { "j", 226, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_k = { "k", 238, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_l = { "l", 250, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_m = { "m", 262, 130, TRUE, MenuItem_CharacterNaming_Add };


MENUITEM gMI_CharacterNaming_n = { "n", 118, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_o = { "o", 130, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_p = { "p", 142, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_q = { "q", 154, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_r = { "r", 166, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_s = { "s", 178, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_t = { "t", 190, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_u = { "u", 202, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_v = { "v", 214, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_w = { "w", 226, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_x = { "x", 238, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_y = { "y", 250, 140, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_z = { "z", 262, 140, TRUE, MenuItem_CharacterNaming_Add };



MENUITEM gMI_CharacterNaming_Back = { "Back", 118, 150, TRUE, MenuItem_CharacterNaming_Back };

MENUITEM gMI_CharacterNaming_OK = { "OK", 256, 150, TRUE, MenuItem_CharacterNaming_OK };


MENUITEM* gMI_CharacterNamingItems[] = {
	&gMI_CharacterNaming_A, &gMI_CharacterNaming_B, &gMI_CharacterNaming_C, &gMI_CharacterNaming_D, &gMI_CharacterNaming_E, &gMI_CharacterNaming_F,
	&gMI_CharacterNaming_G, &gMI_CharacterNaming_H, &gMI_CharacterNaming_I, &gMI_CharacterNaming_J, &gMI_CharacterNaming_K, &gMI_CharacterNaming_L,
	&gMI_CharacterNaming_M, &gMI_CharacterNaming_N, &gMI_CharacterNaming_O, &gMI_CharacterNaming_P, &gMI_CharacterNaming_Q, &gMI_CharacterNaming_R,
	&gMI_CharacterNaming_S, &gMI_CharacterNaming_T, &gMI_CharacterNaming_U, &gMI_CharacterNaming_V, &gMI_CharacterNaming_W, &gMI_CharacterNaming_X,
	&gMI_CharacterNaming_Y, &gMI_CharacterNaming_Z,
	&gMI_CharacterNaming_a, &gMI_CharacterNaming_b, &gMI_CharacterNaming_c, &gMI_CharacterNaming_d, &gMI_CharacterNaming_e, &gMI_CharacterNaming_f,
	&gMI_CharacterNaming_g, &gMI_CharacterNaming_h, &gMI_CharacterNaming_i, &gMI_CharacterNaming_j, &gMI_CharacterNaming_k, &gMI_CharacterNaming_l,
	&gMI_CharacterNaming_m, &gMI_CharacterNaming_n, &gMI_CharacterNaming_o, &gMI_CharacterNaming_p, &gMI_CharacterNaming_q, &gMI_CharacterNaming_r,
	&gMI_CharacterNaming_s, &gMI_CharacterNaming_t, &gMI_CharacterNaming_u, &gMI_CharacterNaming_v, &gMI_CharacterNaming_w, &gMI_CharacterNaming_x,
	&gMI_CharacterNaming_y, &gMI_CharacterNaming_z, &gMI_CharacterNaming_Back, &gMI_CharacterNaming_OK };

MENU gMenu_CharacterNaming = { "What's your name, hero?", 0, _countof(gMI_CharacterNamingItems), gMI_CharacterNamingItems };

void DrawCharacterNaming(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;

    static int16_t BrightnessAdjustment = -255;

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));

        BrightnessAdjustment = -255;

        gMenu_CharacterNaming.SelectedItem = 0;

        gInputEnabled = FALSE;
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter == 10)
    {
        TextColor.Red = 64;

        TextColor.Green = 64;

        TextColor.Blue = 64;

        BrightnessAdjustment = -128;
    }

    if (LocalFrameCounter == 20)
    {
        TextColor.Red = 128;

        TextColor.Green = 128;

        TextColor.Blue = 128;

        BrightnessAdjustment = -64;
    }

    if (LocalFrameCounter == 30)
    {
        TextColor.Red = 192;

        TextColor.Green = 192;

        TextColor.Blue = 192;

        BrightnessAdjustment = -32;
    }

    if (LocalFrameCounter == 40)
    {
        TextColor.Red = 255;

        TextColor.Green = 255;

        TextColor.Blue = 255;

        BrightnessAdjustment = 0;

        gInputEnabled = TRUE;
    }

    BlitStringToBuffer(gMenu_CharacterNaming.Name, &g6x7Font, &TextColor, (GAME_RES_WIDTH / 2) - (((uint16_t)strlen(gMenu_CharacterNaming.Name) * 6) / 2), 16);

    Blit32BppBitmapToBuffer(&gPlayer.Sprite[SUIT_0][FACING_DOWN_0], 153, 85, BrightnessAdjustment);

    for (uint8_t Letter = 0; Letter < _countof(gPlayer.Name) - 1; Letter++)
    {
        if (gPlayer.Name[Letter] == '\0')
        {
            BlitStringToBuffer("_", &g6x7Font, &TextColor, 173 + (Letter * 6), 93);
        }
        else
        {
            BlitStringToBuffer(&gPlayer.Name[Letter], &g6x7Font, &TextColor, 173 + (Letter * 6), 93);
        }
    }

    for (uint8_t Counter = 0; Counter < gMenu_CharacterNaming.ItemCount; Counter++)
    {
        BlitStringToBuffer(gMenu_CharacterNaming.Items[Counter]->Name,
            &g6x7Font,
            &TextColor,
            gMenu_CharacterNaming.Items[Counter]->x,
            gMenu_CharacterNaming.Items[Counter]->y);
    }

    BlitStringToBuffer("»",
        &g6x7Font,
        &TextColor,
        gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->x - 6,
        gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;

}

void PPI_CharacterNaming(void)
{
    // Character Naming Menu

    // A B C D E F G H I J K L M 
    // N O P Q R S T U V W X Y Z
    // a b c d e f g h i j k l m
    // n o p q r s t u v w x y z
    // Back                   OK


#define BACK_BUTTON 52

#define OK_BUTTON   53

#define CAPITAL_M   12

#define LOWERCASE_N 37

#define LOWERCASE_Z 51

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem > CAPITAL_M)
        {
            if (gMenu_CharacterNaming.SelectedItem == OK_BUTTON)
            {
                gMenu_CharacterNaming.SelectedItem = LOWERCASE_Z;
            }
            else
            {
                gMenu_CharacterNaming.SelectedItem -= 13;
            }
        }
        else
        {
            gMenu_CharacterNaming.SelectedItem = BACK_BUTTON;
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem < 39)
        {
            gMenu_CharacterNaming.SelectedItem += 13;
        }
        else if (gMenu_CharacterNaming.SelectedItem >= LOWERCASE_N)
        {
            gMenu_CharacterNaming.SelectedItem = BACK_BUTTON;
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem == 0 ||
            gMenu_CharacterNaming.SelectedItem == 13 ||
            gMenu_CharacterNaming.SelectedItem == 26 ||
            gMenu_CharacterNaming.SelectedItem == 39)
        {
            gMenu_CharacterNaming.SelectedItem += 12;
        }
        else
        {
            gMenu_CharacterNaming.SelectedItem--;
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.RightKeyIsDown && !gGameInput.RightKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem == 12 ||
            gMenu_CharacterNaming.SelectedItem == 25 ||
            gMenu_CharacterNaming.SelectedItem == 38 ||
            gMenu_CharacterNaming.SelectedItem == LOWERCASE_Z)
        {
            gMenu_CharacterNaming.SelectedItem -= 12;
        }
        else
        {
            if (gMenu_CharacterNaming.SelectedItem < OK_BUTTON)
            {
                gMenu_CharacterNaming.SelectedItem++;
            }
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->Action();
    }

    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        MenuItem_CharacterNaming_Back();
    }
}

void MenuItem_CharacterNaming_Add(void)
{
    if (strlen(gPlayer.Name) < 8)
    {
        gPlayer.Name[strlen(gPlayer.Name)] = gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->Name[0];

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_CharacterNaming_Back(void)
{
    if (strlen(gPlayer.Name) < 1)
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_TITLESCREEN;
    }
    else
    {
        gPlayer.Name[strlen(gPlayer.Name) - 1] = '\0';
    }

    PlayGameSound(&gSoundMenuChoose);
}

void MenuItem_CharacterNaming_OK(void)
{
    if (strlen(gPlayer.Name) > 0)
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_OVERWORLD;

        gPlayer.Active = TRUE;        

        PlayGameSound(&gSoundMenuChoose);
    }
}